/***************************************************************************
**
** Copyright (c) 2013 - 2019 Jolla Ltd.
** Copyright (c) 2019 - 2020 Open Mobile Platform LLC.
**
** This file is part of lipstick.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#ifdef HAVE_CONTENTACTION
#include <contentaction.h>
#endif

#include <QWaylandSeat>
#include <QDesktopServices>
#include <QtSensors/QOrientationSensor>
#include <QClipboard>
#include <QSettings>
#include <QMimeData>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include "homeapplication.h"
#include "touchscreen/touchscreen.h"
#include "windowmodel.h"
#include "lipstickcompositorprocwindow.h"
#include "lipstickcompositor.h"
#include "lipstickcompositoradaptor.h"
#include "fileserviceadaptor.h"
#include "lipsticksettings.h"
#include <qpa/qwindowsysteminterface.h>
#include "hwcrenderstage.h"
#include "logging.h"
#include <private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformintegration.h>
#include <QWaylandQuickShellSurfaceItem>

#include <mce/dbus-names.h>
#include <mce/mode-names.h>
#include <qmcenameowner.h>
#include <dbus/dbus-protocol.h>
#include <sys/types.h>
#include <systemd/sd-bus.h>
#include <systemd/sd-login.h>
#include <unistd.h>

#define MCE_DISPLAY_LPM_SET_SUPPORTED "set_lpm_supported"

LipstickCompositor *LipstickCompositor::m_instance = 0;

LipstickCompositor::LipstickCompositor()
    : m_totalWindowCount(0)
    , m_nextWindowId(1)
    , m_homeActive(true)
    , m_shaderEffect(0)
    , m_fullscreenSurface(0)
    , m_directRenderingActive(false)
    , m_topmostWindowId(0)
    , m_topmostWindowProcessId(0)
    , m_topmostWindowOrientation(Qt::PrimaryOrientation)
    , m_screenOrientation(Qt::PrimaryOrientation)
    , m_sensorOrientation(Qt::PrimaryOrientation)
    , m_retainedSelection(0)
    , m_updatesEnabled(true)
    , m_completed(false)
    , m_onUpdatesDisabledUnfocusedWindowId(0)
    , m_fakeRepaintTriggered(false)
    , m_queuedSetUpdatesEnabledCalls()
    , m_mceNameOwner(new QMceNameOwner(this))
    , m_sessionActivationTries(0)
{
    m_window = new QQuickWindow();
    m_window->setColor(Qt::black);
    m_window->setVisible(true);

    m_output = new QWaylandQuickOutput(this, m_window);
    m_output->setSizeFollowsWindow(true);
    connect(this, &QWaylandCompositor::surfaceCreated, this, &LipstickCompositor::onSurfaceCreated);

    m_xdgShell = new QWaylandXdgShell(this);
    connect(m_xdgShell, &QWaylandXdgShell::toplevelCreated, this, &LipstickCompositor::onToplevelCreated);

    m_wm = new QWaylandQtWindowManager(this);
    connect(m_wm, &QWaylandQtWindowManager::openUrl, this, &LipstickCompositor::openUrl);

    setRetainedSelectionEnabled(true);

    if (m_instance) qFatal("LipstickCompositor: Only one compositor instance per process is supported");
    m_instance = this;

    m_orientationLock = new MGConfItem("/lipstick/orientationLock", this);
    connect(m_orientationLock, SIGNAL(valueChanged()), SIGNAL(orientationLockChanged()));

    // Load legacy settings from the config file and delete it from there
    QSettings legacySettings("nemomobile", "lipstick");
    QString legacyOrientationKey("Compositor/orientationLock");
    if (legacySettings.contains(legacyOrientationKey)) {
        m_orientationLock->set(legacySettings.value(legacyOrientationKey));
        legacySettings.remove(legacyOrientationKey);
    }

    connect(m_window, SIGNAL(visibleChanged(bool)), this, SLOT(onVisibleChanged(bool)));
    QObject::connect(HomeApplication::instance(), SIGNAL(aboutToDestroy()), this, SLOT(homeApplicationAboutToDestroy()));

    m_orientationSensor = new QOrientationSensor(this);
    QObject::connect(m_orientationSensor, SIGNAL(readingChanged()), this, SLOT(setScreenOrientationFromSensor()));
    if (!m_orientationSensor->connectToBackend()) {
        qWarning() << "Could not connect to the orientation sensor backend";
    } else {
        if (!m_orientationSensor->start())
            qWarning() << "Could not start the orientation sensor";
    }
    emit HomeApplication::instance()->homeActiveChanged();

    QDesktopServices::setUrlHandler("http", this, "openUrl");
    QDesktopServices::setUrlHandler("https", this, "openUrl");
    QDesktopServices::setUrlHandler("mailto", this, "openUrl");

    connect(QGuiApplication::clipboard(), SIGNAL(dataChanged()), SLOT(clipboardDataChanged()));

    HwcRenderStage::initialize(this);

    m_timedDbus = new Maemo::Timed::Interface();
    if( !m_timedDbus->isValid() )
    {
        qWarning() << "invalid dbus interface:" << m_timedDbus->lastError();
    }

    QTimer::singleShot(0, this, SLOT(initialize()));

    QObject::connect(m_mceNameOwner, &QMceNameOwner::validChanged,
                     this, &LipstickCompositor::processQueuedSetUpdatesEnabledCalls);
    QObject::connect(m_mceNameOwner, &QMceNameOwner::nameOwnerChanged,
                     this, &LipstickCompositor::processQueuedSetUpdatesEnabledCalls);

    setUpdatesEnabledNow(false);
}

LipstickCompositor::~LipstickCompositor()
{
    // ~QWindow can a call into onVisibleChanged and QWaylandCompositor after we
    // are destroyed, so disconnect it.
    disconnect(m_window, SIGNAL(visibleChanged(bool)), this, SLOT(onVisibleChanged(bool)));

    delete m_timedDbus;
    delete m_shaderEffect;

    m_instance = nullptr;
}

LipstickCompositor *LipstickCompositor::instance()
{
    return m_instance;
}

void LipstickCompositor::homeApplicationAboutToDestroy()
{
    m_window->hide();
    m_window->releaseResources();

    // When destroying LipstickCompositor ~QQuickWindow() is called after
    // ~QWaylandQuickCompositor(), so changes to the items in the window may end
    // up calling code such as LipstickCompositorWindow::handleTouchCancel(),
    // which will try to use the compositor, at that point not usable anymore.
    // So delete all the windows here.
    foreach (LipstickCompositorWindow *w, m_windows) {
        delete w;
    }

    m_instance = 0;
    delete this;
}

void LipstickCompositor::onVisibleChanged(bool visible)
{
    if (!visible) {
        m_output->sendFrameCallbacks();
    }
}

static LipstickCompositorWindow *surfaceWindow(QWaylandSurface *surface)
{
    return surface->views().isEmpty() ? 0 : static_cast<LipstickCompositorWindow *>(surface->views().first()->renderObject());
}

void LipstickCompositor::onToplevelCreated(QWaylandXdgToplevel * topLevel, QWaylandXdgSurface * shellSurface)
{
    QWaylandSurface *surface = shellSurface->surface();
    LipstickCompositorWindow *window = surfaceWindow(surface);

    if(!topLevel->fullscreen()) {
        qDebug() << "Make window fullscreen force";
        topLevel->sendFullscreen(QGuiApplication::primaryScreen()->size());
    }

    if(window) {
        connect(topLevel, &QWaylandXdgToplevel::titleChanged, this, &LipstickCompositor::surfaceTitleChanged);
        connect(topLevel, &QWaylandXdgToplevel::setFullscreen, this, &LipstickCompositor::surfaceSetFullScreen);
    }
}

void LipstickCompositor::onSurfaceCreated(QWaylandSurface *surface)
{
    LipstickCompositorWindow *item = surfaceWindow(surface);
    if (!item)
        item = createView(surface);
    connect(surface, SIGNAL(hasContentChanged()), this, SLOT(onHasContentChanged()));
    connect(surface, SIGNAL(damaged(QRegion)), this, SLOT(surfaceDamaged(QRegion)));
    connect(surface, SIGNAL(redraw()), this, SLOT(windowSwapped()));
}

bool LipstickCompositor::openUrl(QWaylandClient *client, const QUrl &url)
{
    Q_UNUSED(client)
#if defined(HAVE_CONTENTACTION)
    const bool isFile = url.scheme() == QLatin1String("file");
    ContentAction::Action defaultAction = isFile
            ? ContentAction::Action::defaultActionForFile(url)
            : ContentAction::Action::defaultActionForScheme(url.toString());

    static const QMetaMethod requestSignal = QMetaMethod::fromSignal(
                &LipstickCompositor::openUrlRequested);
    if (isSignalConnected(requestSignal)) {
        openUrlRequested(
                    url,
                    defaultAction,
                    isFile
                        ? ContentAction::Action::actionsForFile(url)
                        : ContentAction::Action::actionsForUrl(url.toString()));
        return true;
    } else if (defaultAction.isValid()) {
        defaultAction.trigger();
    }

    return defaultAction.isValid();
#else
    Q_UNUSED(url)
    return false;
#endif
}

void LipstickCompositor::retainedSelectionReceived(QMimeData *mimeData)
{
    if (!m_retainedSelection)
        m_retainedSelection = new QMimeData;

    // Make a copy to allow QClipboard to take ownership of our data
    m_retainedSelection->clear();
    foreach (const QString &format, mimeData->formats())
        m_retainedSelection->setData(format, mimeData->data(format));

    QGuiApplication::clipboard()->setMimeData(m_retainedSelection.data());
}

int LipstickCompositor::windowCount() const
{
    return m_mappedSurfaces.count();
}

int LipstickCompositor::ghostWindowCount() const
{
    return m_totalWindowCount - windowCount();
}

bool LipstickCompositor::homeActive() const
{
    return m_homeActive;
}

void LipstickCompositor::setHomeActive(bool a)
{
    if (a == m_homeActive)
        return;

    m_homeActive = a;

    emit homeActiveChanged();
    emit HomeApplication::instance()->homeActiveChanged();
}

bool LipstickCompositor::debug() const
{
    static enum { Yes, No, Unknown } status = Unknown;
    if (status == Unknown) {
        QByteArray v = qgetenv("LIPSTICK_COMPOSITOR_DEBUG");
        bool value = !v.isEmpty() && v != "0" && v != "false";
        if (value) status = Yes;
        else status = No;
    }
    return status == Yes;
}

QObject *LipstickCompositor::windowForId(int id) const
{
    QObject *window = m_windows.value(id, NULL);
    return window;
}

void LipstickCompositor::closeClientForWindowId(int id)
{
    LipstickCompositorWindow *window = m_windows.value(id, 0);
    if (window && window->surface())
        destroyClientForSurface(window->surface());
}

QWaylandSurface *LipstickCompositor::surfaceForId(int id) const
{
    LipstickCompositorWindow *window = m_windows.value(id, 0);
    return window ? window->surface() : 0;
}

bool LipstickCompositor::completed()
{
    return m_completed;
}

void LipstickCompositor::clearKeyboardFocus()
{
    defaultSeat()->setKeyboardFocus(NULL);
}

void LipstickCompositor::setDisplayOff()
{
    HomeApplication::instance()->setDisplayOff();
}

void LipstickCompositor::surfaceDamaged(const QRegion &)
{
    if (!m_window->isVisible()) {
        // If the compositor is not visible, do not throttle.
        // make it conditional to QT_WAYLAND_COMPOSITOR_NO_THROTTLE?
        m_output->sendFrameCallbacks();
    }
}

void LipstickCompositor::setFullscreenSurface(QWaylandSurface *surface)
{
    if (surface == m_fullscreenSurface)
        return;

    // Prevent flicker when returning to composited mode
    if (!surface && m_fullscreenSurface) {
        foreach (QWaylandView *view, m_fullscreenSurface->views())
            static_cast<LipstickCompositorWindow *>(view->renderObject())->update();
    }

    m_fullscreenSurface = surface;

    emit fullscreenSurfaceChanged();
}

QObject *LipstickCompositor::clipboard() const
{
    return QGuiApplication::clipboard();
}

void LipstickCompositor::setTopmostWindowId(int id)
{
    if (id != m_topmostWindowId) {
        m_topmostWindowId = id;
        emit topmostWindowIdChanged();

        int pid = -1;
        QWaylandSurface *surface = surfaceForId(m_topmostWindowId);

        if (surface)
            pid = surface->client()->processId();

        if (m_topmostWindowProcessId != pid) {
            m_topmostWindowProcessId = pid;
            emit privateTopmostWindowProcessIdChanged(m_topmostWindowProcessId);
        }
    }
}

LipstickCompositorWindow *LipstickCompositor::createView(QWaylandSurface *surface)
{
    int id = m_nextWindowId++;
    LipstickCompositorWindow *item = new LipstickCompositorWindow(id, "", surface, m_window->contentItem());
    QObject::connect(item, SIGNAL(destroyed(QObject*)), this, SLOT(windowDestroyed()));
    m_windows.insert(item->windowId(), item);
    return item;
}

void LipstickCompositor::onSurfaceDying()
{
    QWaylandSurface *surface = static_cast<QWaylandSurface *>(sender());
    LipstickCompositorWindow *item = surfaceWindow(surface);

    if (surface == m_fullscreenSurface)
        setFullscreenSurface(0);

    if (item) {
        item->m_windowClosed = true;
        item->tryRemove();
    }
}

void LipstickCompositor::activateLogindSession()
{
    m_sessionActivationTries++;

    if (m_logindSession.isEmpty()) {
        /* Find the current session based on uid */
        uid_t uid = getuid();
        char **sessions = NULL;
        uid_t *uids = NULL;
        uint count = 0;
        if (sd_seat_get_sessions("seat0", &sessions, &uids, &count) > 0) {
            for (uint i = 0; i < count; ++i) {
                if (uids[i] == uid) {
                    m_logindSession = sessions[i];
                    break;
                }
            }
            for (char **s = sessions; *s ; ++s)
                free(*s);
        }
        free(sessions);
        free(uids);

        if (m_logindSession.isEmpty()) {
            qCWarning(lcLipstickCoreLog) << "Could not read session id, could not activate session";
            return;
        }
    }

    if (sd_session_is_active(m_logindSession.toUtf8()) > 0) {
        qCInfo(lcLipstickCoreLog) << "Session" << m_logindSession << "successfully activated";
        return;
    }

    if (m_sessionActivationTries > 10) {
        qCWarning(lcLipstickCoreLog) << "Could not activate session, giving up";
        return;
    }

    QDBusInterface logind(QStringLiteral("org.freedesktop.login1"),
                          QStringLiteral("/org/freedesktop/login1"),
                          QStringLiteral("org.freedesktop.login1.Manager"),
                          QDBusConnection::systemBus());

    if (!logind.isValid()) {
        qCWarning(lcLipstickCoreLog) << "Logind manager is not a valid interface, could not activate session";
        return;
    }

    qCDebug(lcLipstickCoreLog) << "Activating session on seat0";

    QDBusPendingCall call = logind.asyncCall("ActivateSession", m_logindSession);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *call) {
        QDBusPendingReply<void> reply = *call;
        if (reply.isError()) {
            qCWarning(lcLipstickCoreLog) << "Could not activate session:" << reply.error();
        } else {
            // VT switching may fail without notice, check status again a bit later
            QTimer::singleShot(100, this, &LipstickCompositor::activateLogindSession);
        }
        call->deleteLater();
    });

    qCDebug(lcLipstickCoreLog) << "Session" << m_logindSession << "is activating";
}

void LipstickCompositor::initialize()
{
    activateLogindSession();

    TouchScreen *touchScreen = HomeApplication::instance()->touchScreen();
    reactOnDisplayStateChanges(TouchScreen::DisplayUnknown, touchScreen->currentDisplayState());
    connect(touchScreen, &TouchScreen::displayStateChanged, this, &LipstickCompositor::reactOnDisplayStateChanges);

    new LipstickCompositorAdaptor(this);

    QDBusConnection systemBus = QDBusConnection::systemBus();

    if (!systemBus.registerObject("/", this)) {
        qWarning("Unable to register object at path /: %s", systemBus.lastError().message().toUtf8().constData());
    }

    /* We might have for example minui based encryption unlock ui
     * running as compositor and waiting to be able to hand-off
     * to us -> use ReplaceExistingService to facilitate this.
     */
    QDBusReply<QDBusConnectionInterface::RegisterServiceReply> reply =
            systemBus.interface()->registerService(QStringLiteral("org.nemomobile.compositor"),
                                                   QDBusConnectionInterface::ReplaceExistingService,
                                                   QDBusConnectionInterface::DontAllowReplacement);
    if (!reply.isValid()) {
        qWarning("Unable to register D-Bus service org.nemomobile.compositor: %s",
                 reply.error().message().toUtf8().constData());
    } else if (reply.value() != QDBusConnectionInterface::ServiceRegistered) {
        qWarning("Unable to register D-Bus service org.nemomobile.compositor: %s",
                 "Did not get primary name ownership");
    }


    QDBusMessage message = QDBusMessage::createMethodCall(MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_DISPLAY_LPM_SET_SUPPORTED);
    message.setArguments(QVariantList() << ambientSupported());
    QDBusConnection::systemBus().asyncCall(message);

    new FileServiceAdaptor(this);
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    sessionBus.registerObject(QLatin1String("/"), this);
    sessionBus.registerService(QLatin1String("org.nemomobile.fileservice"));
}

void LipstickCompositor::windowDestroyed(LipstickCompositorWindow *item)
{
    int id = item->windowId();

    m_windows.remove(id);
    surfaceUnmapped(item);
}

void LipstickCompositor::onHasContentChanged()
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());

    if(surface->isCursorSurface())
        return;

    if(surface->hasContent())
        surfaceMapped(surface);
    else
        surfaceUnmapped(surface);
}

void LipstickCompositor::surfaceMapped(QWaylandSurface *surface)
{
    LipstickCompositorWindow *item = surfaceWindow(surface);
    if (!item)
        item = createView(surface);

    // The surface was mapped for the first time
    if (item->m_mapped)
        return;

    QVariantMap properties = item->windowProperties();

    item->m_mapped = true;
    item->m_category = properties.value("CATEGORY").toString();

    if (!item->parentItem()) {
        // TODO why contentItem?
        item->setParentItem(m_window->contentItem());
    }

    QObject::connect(surface, &QWaylandSurface::surfaceDestroyed, this, &LipstickCompositor::onSurfaceDying);

    m_totalWindowCount++;
    m_mappedSurfaces.insert(item->windowId(), item);

    item->setTouchEventsEnabled(true);

    emit windowCountChanged();
    emit windowAdded(item);

    windowAdded(item->windowId());

    emit availableWinIdsChanged();
}

void LipstickCompositor::surfaceTitleChanged()
{
    QWaylandXdgToplevel *xdgShellSurface = qobject_cast<QWaylandXdgToplevel*>(sender());
    LipstickCompositorWindow *window = surfaceWindow(xdgShellSurface->xdgSurface()->surface());
    if (window) {
        window->setTitle(xdgShellSurface->title());
        emit window->titleChanged();

        int windowId = window->windowId();

        for (int ii = 0; ii < m_windowModels.count(); ++ii)
            m_windowModels.at(ii)->titleChanged(windowId);
    }
}

void LipstickCompositor::surfaceSetFullScreen(QWaylandOutput *output)
{
    QWaylandXdgToplevel *xdgShellSurface = qobject_cast<QWaylandXdgToplevel*>(sender());

    QWaylandOutput *designatedOutput = output ? output : m_output;
    if (!designatedOutput)
        return;

    xdgShellSurface->sendFullscreen(designatedOutput->geometry().size() / designatedOutput->scaleFactor());
}

void LipstickCompositor::windowSwapped()
{
    m_output->sendFrameCallbacks();
}

void LipstickCompositor::windowDestroyed()
{
    m_totalWindowCount--;
    m_windows.remove(static_cast<LipstickCompositorWindow *>(sender())->windowId());
    emit ghostWindowCountChanged();
}

void LipstickCompositor::windowPropertyChanged(const QString &property)
{
    qWarning() << "NOT IMPLEMENTED: Window properties changed:" << property;
    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());
    LipstickCompositorWindow *window = surfaceWindow(surface);
    if(window) {
        if (property == QLatin1String("MOUSE_REGION")) {
            window->refreshMouseRegion();
        } else if (property == QLatin1String("GRABBED_KEYS")) {
            window->refreshGrabbedKeys();
        }
    }

}

void LipstickCompositor::surfaceUnmapped(QWaylandSurface *surface)
{
    if (surface == m_fullscreenSurface)
        setFullscreenSurface(0);

    LipstickCompositorWindow *window = surfaceWindow(surface);
    if (window)
        emit windowHidden(window);
}

void LipstickCompositor::surfaceUnmapped(LipstickCompositorWindow *item)
{
    int id = item->windowId();

    if (m_mappedSurfaces.remove(id) == 0) {
        // It was unmapped already so nothing to do
        return;
    }

    emit windowCountChanged();
    emit windowRemoved(item);

    item->m_windowClosed = true;
    item->tryRemove();

    emit ghostWindowCountChanged();

    windowRemoved(id);

    emit availableWinIdsChanged();
}

void LipstickCompositor::windowAdded(int id)
{
    for (int ii = 0; ii < m_windowModels.count(); ++ii)
        m_windowModels.at(ii)->addItem(id);
}

void LipstickCompositor::windowRemoved(int id)
{
    for (int ii = 0; ii < m_windowModels.count(); ++ii)
        m_windowModels.at(ii)->remItem(id);
}

QQmlComponent *LipstickCompositor::shaderEffectComponent()
{
    const char *qml_source =
            "import QtQuick 2.0\n"
            "ShaderEffect {\n"
            "property QtObject window\n"
            "property ShaderEffectSource source: ShaderEffectSource { sourceItem: window }\n"
            "}";

    if (!m_shaderEffect) {
        m_shaderEffect = new QQmlComponent(qmlEngine(this));
        m_shaderEffect->setData(qml_source, QUrl());
    }
    return m_shaderEffect;
}

void LipstickCompositor::setTopmostWindowOrientation(Qt::ScreenOrientation topmostWindowOrientation)
{
    if (m_topmostWindowOrientation != topmostWindowOrientation) {
        m_topmostWindowOrientation = topmostWindowOrientation;
        emit topmostWindowOrientationChanged();
    }
}

void LipstickCompositor::setScreenOrientation(Qt::ScreenOrientation screenOrientation)
{
    if (m_screenOrientation != screenOrientation) {
        if (debug())
            qDebug() << "Setting screen orientation on QWaylandCompositor";

        QSize physSize = m_output->physicalSize();
        switch(screenOrientation) {
        case Qt::PrimaryOrientation:
            m_output->setTransform(QWaylandOutput::TransformNormal);
            break;
        case Qt::LandscapeOrientation:
            if(physSize.width() > physSize.height())
                m_output->setTransform(QWaylandOutput::TransformNormal);
            else
                m_output->setTransform(QWaylandOutput::Transform90);
            break;
        case Qt::PortraitOrientation:
            if(physSize.width() > physSize.height())
                m_output->setTransform(QWaylandOutput::Transform90);
            else
                m_output->setTransform(QWaylandOutput::TransformNormal);
            break;
        case Qt::InvertedLandscapeOrientation:
            if(physSize.width() > physSize.height())
                m_output->setTransform(QWaylandOutput::Transform180);
            else
                m_output->setTransform(QWaylandOutput::Transform270);
            break;
        case Qt::InvertedPortraitOrientation:
            if(physSize.width() > physSize.height())
                m_output->setTransform(QWaylandOutput::Transform270);
            else
                m_output->setTransform(QWaylandOutput::Transform180);
            break;
        }
        QWindowSystemInterface::handleScreenOrientationChange(qApp->primaryScreen(),screenOrientation);

        m_screenOrientation = screenOrientation;
        emit screenOrientationChanged();
    }
}

bool LipstickCompositor::displayDimmed() const
{
    TouchScreen *touchScreen = HomeApplication::instance()->touchScreen();
    return touchScreen->currentDisplayState() == TouchScreen::DisplayDimmed;
}

void LipstickCompositor::reactOnDisplayStateChanges(TouchScreen::DisplayState oldState, TouchScreen::DisplayState newState)
{
    if (newState == TouchScreen::DisplayOn) {
        emit displayOn();
    } else if (newState == TouchScreen::DisplayOff) {
        QCoreApplication::postEvent(this, new QTouchEvent(QEvent::TouchCancel));
        emit displayOff();
    }

    bool changeInDimming = (newState == TouchScreen::DisplayDimmed) != (oldState == TouchScreen::DisplayDimmed);

    bool changeInAmbient = ((newState == TouchScreen::DisplayOff) != (oldState == TouchScreen::DisplayOff)) && ambientEnabled();

    bool enterAmbient = changeInAmbient && (newState == TouchScreen::DisplayOff);
    bool leaveAmbient = changeInAmbient && (newState != TouchScreen::DisplayOff);

    oldState = newState;

    if (changeInDimming) {
        emit displayDimmedChanged();
    }
    if (changeInAmbient) {
        emit displayAmbientChanged();
    }
    if (enterAmbient) {
        emit displayAmbientEntered();
    }
    if (leaveAmbient) {
        emit displayAmbientLeft();
    }
}

void LipstickCompositor::setScreenOrientationFromSensor()
{
    QOrientationReading* reading = m_orientationSensor->reading();

    if (debug())
        qDebug() << "Screen orientation changed " << reading->orientation();

    Qt::ScreenOrientation sensorOrientation = m_sensorOrientation;
    switch (reading->orientation()) {
    case QOrientationReading::TopUp:
        sensorOrientation = Qt::PortraitOrientation;
        break;
    case QOrientationReading::TopDown:
        sensorOrientation = Qt::InvertedPortraitOrientation;
        break;
    case QOrientationReading::LeftUp:
        sensorOrientation = Qt::InvertedLandscapeOrientation;
        break;
    case QOrientationReading::RightUp:
        sensorOrientation = Qt::LandscapeOrientation;
        break;
    case QOrientationReading::FaceUp:
    case QOrientationReading::FaceDown:
        /* Keep screen orientation at previous state */
        break;
    case QOrientationReading::Undefined:
    default:
        sensorOrientation = Qt::PrimaryOrientation;
        break;
    }

    if (sensorOrientation != m_sensorOrientation) {
        m_sensorOrientation = sensorOrientation;
        emit sensorOrientationChanged();
    }
}

void LipstickCompositor::clipboardDataChanged()
{
    const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData();
    if (mimeData && mimeData != m_retainedSelection)
        overrideSelection(const_cast<QMimeData *>(mimeData));
}

bool LipstickCompositor::ambientSupported() const
{
    return QGuiApplication::platformNativeInterface()->nativeResourceForIntegration("AmbientSupported");
}

void LipstickCompositor::setAmbientEnabled(bool enabled)
{
    if (!ambientSupported()) {
        return;
    }

    if (m_ambientModeEnabled == enabled) {
        return;
    }

    m_ambientModeEnabled = enabled;
    if (m_ambientModeEnabled) {
        QGuiApplication::platformNativeInterface()->nativeResourceForIntegration("AmbientEnable");
    } else {
        QGuiApplication::platformNativeInterface()->nativeResourceForIntegration("AmbientDisable");
    }
    emit ambientEnabledChanged();
}

void LipstickCompositor::scheduleAmbientUpdate()
{
    if (!ambientEnabled()) {
        return;
    }
    QMap<QString,QVariant> match;
    match.insert("type", QVariant(QString("wakeup")));
    QDBusReply< QList<QVariant> > reply = m_timedDbus->query_sync(match);

    if( !reply.isValid() ) {
        qWarning() << "'query' call failed:" << m_timedDbus->lastError();
        return;
    }

    uint cookie = 0;
    QList<QVariant> cookies = reply.value();
    // Cancel all wakeup cookies except one.
    while (!cookies.isEmpty()) {
        bool ok = true;
        cookie = cookies.takeFirst().toUInt(&ok);
        if (!ok) {
            cookie = 0;
            continue;
        }
        // If the current cookie isn't the last one in the list.
        if (!cookies.isEmpty()) {
            QDBusReply<bool> res = m_timedDbus->cancel_sync(cookie);
            if (!res.isValid()) {
                qWarning() << "'cancel' call failed:" << m_timedDbus->lastError();
            } else {
                qWarning() << "cookie " << cookie << " deleted " << res.value();
            }
        }
    }

    // Add new wakeup event
    Maemo::Timed::Event wakeupEvent;

    time_t currentTime;
    struct tm* timeinfo;
    time(&currentTime);
    // We don't want to update the screen 60 seconds after the screen is off.
    // The screen should be updated when the minute digit changes.
    timeinfo = localtime(&currentTime);
    timeinfo->tm_sec = 0;

    time_t wakeupTime = mktime(timeinfo);
    if (wakeupTime == -1) {
        wakeupTime = currentTime;
    }
    wakeupTime += 60;
    wakeupEvent.setTicker(wakeupTime);
    wakeupEvent.setAttribute(QLatin1String("APPLICATION"), QLatin1String("wakup_alarm"));
    wakeupEvent.setAttribute(QLatin1String("type"), QLatin1String("wakeup"));
    wakeupEvent.setBootFlag();
    wakeupEvent.setKeepAliveFlag();
    wakeupEvent.setReminderFlag();
    wakeupEvent.setAlarmFlag();
    wakeupEvent.setSingleShotFlag();

    if (cookie) {
        QDBusReply<uint> res = m_timedDbus->replace_event_sync(wakeupEvent, cookie);
    } else {
        QDBusReply<uint> res = m_timedDbus->add_event_sync(wakeupEvent);
    }
}

void LipstickCompositor::setAmbientUpdatesEnabled(bool enabled)
{
    TouchScreen *touchScreen = HomeApplication::instance()->touchScreen();
    if (enabled) {
        if (touchScreen->currentDisplayState() == TouchScreen::DisplayOn) {
            return;
        }
        if (!ambientEnabled()) {
            return;
        }
    }
    setUpdatesEnabled(enabled, true);;
    if (enabled) {
        emit displayAmbientUpdate();
    }
}

bool LipstickCompositor::displayAmbient() const
{
    TouchScreen *touchScreen = HomeApplication::instance()->touchScreen();
    return touchScreen->currentDisplayState() == TouchScreen::DisplayOn;
}

void LipstickCompositor::setUpdatesEnabledNow(bool enabled, bool inAmbientMode)
{
    if ((m_updatesEnabled != enabled) || inAmbientMode) {
        if (!inAmbientMode) {
            m_updatesEnabled = enabled;
        }
        if (!enabled) {
            emit displayAboutToBeOff();
            LipstickCompositorWindow *topmostWindow = qobject_cast<LipstickCompositorWindow *>(windowForId(topmostWindowId()));
            if (topmostWindow != 0 && topmostWindow->hasFocus()) {
                m_onUpdatesDisabledUnfocusedWindowId = topmostWindow->windowId();
                clearKeyboardFocus();
            }
            m_window->hide();
            if (m_window->handle() && !inAmbientMode) {
                QGuiApplication::platformNativeInterface()->nativeResourceForIntegration("DisplayOff");
            }
            // trigger frame callbacks which are pending already at this time
            surfaceCommitted();

            scheduleAmbientUpdate();
        } else {
            if (m_window->handle() && !inAmbientMode) {
                QGuiApplication::platformNativeInterface()->nativeResourceForIntegration("DisplayOn");
            }
            emit displayAboutToBeOn();
            m_window->showFullScreen();
            if (m_onUpdatesDisabledUnfocusedWindowId > 0) {
                if (!LipstickSettings::instance()->lockscreenVisible()) {
                    LipstickCompositorWindow *topmostWindow = qobject_cast<LipstickCompositorWindow *>(windowForId(topmostWindowId()));
                    if (topmostWindow != 0 && topmostWindow->windowId() == m_onUpdatesDisabledUnfocusedWindowId) {
                        topmostWindow->takeFocus();
                    }
                }
                m_onUpdatesDisabledUnfocusedWindowId = 0;
            }
        }
    }

    if (enabled && !m_completed) {
        m_completed = true;
        emit completedChanged();
    }
}

void LipstickCompositor::setUpdatesEnabled(bool enabled, bool inAmbientMode)
{
    if (!calledFromDBus()) {
        setUpdatesEnabledNow(enabled, inAmbientMode);
    } else {
        if (message().isReplyRequired())
            setDelayedReply(true);
        m_queuedSetUpdatesEnabledCalls.append(QueuedSetUpdatesEnabledCall(connection(), message(), enabled));
        QMetaObject::invokeMethod(this, "processQueuedSetUpdatesEnabledCalls", Qt::QueuedConnection);
    }
}

void LipstickCompositor::processQueuedSetUpdatesEnabledCalls()
{
    if (m_mceNameOwner->valid()) {
        while (!m_queuedSetUpdatesEnabledCalls.isEmpty()) {
            QueuedSetUpdatesEnabledCall queued(m_queuedSetUpdatesEnabledCalls.takeFirst());
            if (queued.m_message.service() != m_mceNameOwner->nameOwner()) {
                if (queued.m_message.isReplyRequired()) {
                    QDBusMessage reply(queued.m_message.createErrorReply(DBUS_ERROR_ACCESS_DENIED,
                                                                         "Only mce is allowed to call this method"));
                    queued.m_connection.send(reply);
                }
            } else {
                setUpdatesEnabledNow(queued.m_enable);
                if (queued.m_message.isReplyRequired()) {
                    QDBusMessage reply(queued.m_message.createReply());
                    queued.m_connection.send(reply);
                }
            }
        }
    }
}

void LipstickCompositor::surfaceCommitted()
{
    if (!m_window->isVisible() && !m_fakeRepaintTriggered) {
        startTimer(1000);
        m_fakeRepaintTriggered = true;
    }
}

void LipstickCompositor::timerEvent(QTimerEvent *e)
{
    Q_UNUSED(e)

    m_output->frameStarted();
    m_output->sendFrameCallbacks();
    killTimer(e->timerId());
}
