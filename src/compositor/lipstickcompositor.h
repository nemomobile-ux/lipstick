/***************************************************************************
**
** Copyright (c) 2013-2019 Jolla Ltd.
** Copyright (c) 2019 Open Mobile Platform LLC.
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

#ifndef LIPSTICKCOMPOSITOR_H
#define LIPSTICKCOMPOSITOR_H

#include <QQuickWindow>
#include "lipstickglobal.h"
#include "homeapplication.h"
#include <QQmlParserStatus>
#include <QWaylandQuickCompositor>
#include <QWaylandQuickOutput>
#include <QWaylandWlShellSurface>
#include <QQmlComponent>
#include <QWaylandClient>
#include <QWaylandSurface>
#include <QWaylandKeymap>
#include <QPointer>
#include <QTimer>
#include <MGConfItem>
#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusMessage>

#include <touchscreen.h>

QT_BEGIN_NAMESPACE

class QWaylandXdgShellV5;
class QWaylandXdgSurfaceV5;
class QWaylandOutput;

QT_END_NAMESPACE

class AlienClient;
class AlienManager;
class AlienSurface;
class WindowModel;
class WindowPropertyMap;
class LipstickCompositorWindow;
class LipstickCompositorProcWindow;
class QOrientationSensor;
class LipstickRecorderManager;
class LipstickKeymap;
class QMceNameOwner;

namespace ContentAction {
class Action;
}

struct QueuedSetUpdatesEnabledCall
{
    QueuedSetUpdatesEnabledCall(const QDBusConnection &connection, const QDBusMessage &message, bool enable)
    : m_connection(connection)
    , m_message(message)
    , m_enable(enable)
    {
    }

    QDBusConnection m_connection;
    QDBusMessage m_message;
    bool m_enable;
};

class LIPSTICK_EXPORT LipstickCompositor 
	: public QWaylandQuickCompositor
	, public QDBusContext
{
    Q_OBJECT

    Q_PROPERTY(int windowCount READ windowCount NOTIFY windowCountChanged)
    Q_PROPERTY(int ghostWindowCount READ ghostWindowCount NOTIFY ghostWindowCountChanged)
    Q_PROPERTY(bool homeActive READ homeActive WRITE setHomeActive NOTIFY homeActiveChanged)
    Q_PROPERTY(bool debug READ debug CONSTANT)
    Q_PROPERTY(QWaylandSurface* fullscreenSurface READ fullscreenSurface WRITE setFullscreenSurface NOTIFY fullscreenSurfaceChanged)
    Q_PROPERTY(bool directRenderingActive READ directRenderingActive NOTIFY directRenderingActiveChanged)
    Q_PROPERTY(int topmostWindowId READ topmostWindowId WRITE setTopmostWindowId NOTIFY topmostWindowIdChanged)
    Q_PROPERTY(Qt::ScreenOrientation topmostWindowOrientation READ topmostWindowOrientation WRITE setTopmostWindowOrientation NOTIFY topmostWindowOrientationChanged)
    Q_PROPERTY(Qt::ScreenOrientation screenOrientation READ screenOrientation WRITE setScreenOrientation NOTIFY screenOrientationChanged)
    Q_PROPERTY(Qt::ScreenOrientation sensorOrientation READ sensorOrientation NOTIFY sensorOrientationChanged)
    Q_PROPERTY(QObject* clipboard READ clipboard CONSTANT)
    Q_PROPERTY(QVariant orientationLock READ orientationLock NOTIFY orientationLockChanged)
    Q_PROPERTY(bool displayDimmed READ displayDimmed NOTIFY displayDimmedChanged)
    Q_PROPERTY(bool completed READ completed NOTIFY completedChanged)
    Q_PROPERTY(QQuickWindow *quickWindow READ quickWindow CONSTANT)
    Q_PROPERTY(QQuickItem *contentItem READ contentItem CONSTANT)
    Q_PROPERTY(QQmlListProperty<QObject> data READ data)
    Q_PROPERTY(QWaylandKeymap *keymap READ keymap CONSTANT)
    Q_CLASSINFO("DefaultProperty", "data")
public:
    LipstickCompositor();
    ~LipstickCompositor();

    static LipstickCompositor *instance();

    bool openUrl(QWaylandClient *client, const QUrl &url);
    void retainedSelectionReceived(QMimeData *mimeData) Q_DECL_OVERRIDE;

    int windowCount() const;
    int ghostWindowCount() const;

    bool homeActive() const;
    void setHomeActive(bool);

    QWaylandSurface *fullscreenSurface() const { return m_fullscreenSurface; }
    void setFullscreenSurface(QWaylandSurface *surface);
    bool directRenderingActive() const { return m_directRenderingActive; }

    int topmostWindowId() const { return m_topmostWindowId; }
    void setTopmostWindowId(int id);
    int privateTopmostWindowProcessId() const { return m_topmostWindowProcessId; }

    Qt::ScreenOrientation topmostWindowOrientation() const { return m_topmostWindowOrientation; }
    void setTopmostWindowOrientation(Qt::ScreenOrientation topmostWindowOrientation);

    Qt::ScreenOrientation screenOrientation() const { return m_screenOrientation; }
    void setScreenOrientation(Qt::ScreenOrientation screenOrientation);

    Qt::ScreenOrientation sensorOrientation() const { return m_sensorOrientation; }

    QVariant orientationLock() const { return m_orientationLock->value("dynamic"); }

    bool displayDimmed() const;

    QObject *clipboard() const;

    bool debug() const;

    Q_INVOKABLE LipstickCompositorWindow *windowForId(int) const;
    Q_INVOKABLE void closeClientForWindowId(int);
    Q_INVOKABLE void clearKeyboardFocus();
    Q_INVOKABLE void setDisplayOff();
    Q_INVOKABLE QVariant settingsValue(const QString &key, const QVariant &defaultValue = QVariant()) const
        { return (key == "orientationLock") ? m_orientationLock->value(defaultValue) : MGConfItem("/lipstick/" + key).value(defaultValue); }

    LipstickCompositorProcWindow *mapProcWindow(const QString &title, const QString &category, const QRect &);
    LipstickCompositorProcWindow *mapProcWindow(const QString &title, const QString &category, const QRect &, QQuickItem *rootItem);

    QWaylandSurface *surfaceForId(int) const;

    bool completed();

    void setUpdatesEnabledNow(bool enabled);
    void setUpdatesEnabled(bool enabled);

    QQuickWindow *quickWindow() { return m_window.data(); }
    void setQuickWindow(QQuickWindow *window);

    QQuickItem *contentItem() { return m_window->contentItem(); }
    QQmlListProperty<QObject> data();
    QWaylandKeymap *keymap();

    static LipstickCompositorWindow *surfaceWindow(QWaylandSurface *surface);

protected:
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;
    void componentComplete() override;

signals:
    void windowAdded(QObject *window);
    void windowRemoved(QObject *window);
    void windowRaised(QObject *window);
    void windowLowered(QObject *window);
    void windowHidden(QObject *window);

    void windowCountChanged();
    void ghostWindowCountChanged();

    void availableWinIdsChanged();

    void homeActiveChanged();
    void fullscreenSurfaceChanged();
    void directRenderingActiveChanged();
    void topmostWindowIdChanged();
    void privateTopmostWindowProcessIdChanged(int pid);
    void topmostWindowOrientationChanged();
    void screenOrientationChanged();
    void sensorOrientationChanged();
    void orientationLockChanged();
    void displayDimmedChanged();

    void displayOn();
    void displayOff();
    void displayAboutToBeOn();
    void displayAboutToBeOff();

    void completedChanged();

    void showUnlockScreen();


    void openUrlRequested(
            const QUrl &url,
            const ContentAction::Action &defaultAction,
            const QList<ContentAction::Action> &candidateActions);

private slots:
    void surfaceDamaged(const QRegion &);
    void windowSwapped();
    bool openUrl(const QUrl &);
    void reactOnDisplayStateChanges(TouchScreen::DisplayState oldState, TouchScreen::DisplayState newState);
    void homeApplicationAboutToDestroy();
    void setScreenOrientationFromSensor();
    void clipboardDataChanged();
    void onVisibleChanged(bool visible);
    void initialize();
    void processQueuedSetUpdatesEnabledCalls();

    void onShellSurfaceCreated(QWaylandWlShellSurface *wlShellSurface);
    void onXdgSurfaceCreated(QWaylandXdgSurfaceV5 *xdgSurface);
    void onAlienSurfaceCreated(AlienSurface *alienSurface, QWaylandSurface *surface);

private:
    friend class AlienClient;
    friend class LipstickCompositorWindow;
    friend class LipstickCompositorProcWindow;
    friend class WindowModel;
    friend class WindowPropertyMap;
    friend class NotificationPreviewPresenter;
    friend class NotificationFeedbackPlayer;

    void surfaceMapped(LipstickCompositorWindow *window, QWaylandSurface *surface);
    void surfaceUnmapped(LipstickCompositorWindow *window, QWaylandSurface *surface);

    int windowIdForLink(QWaylandSurface *, uint) const;
    void windowAdded(int);
    void windowRemoved(int);
    void windowDestroyed(LipstickCompositorWindow *item);
    void readContent();
    void surfaceCommitted();

    QQmlComponent *shaderEffectComponent();



    static LipstickCompositor *m_instance;
    static inline void data_append(QQmlListProperty<QObject> *property, QObject *object);
    static inline int data_count(QQmlListProperty<QObject> *property);
    static inline QObject *data_at(QQmlListProperty<QObject> *property, int index);
    static inline void data_clear(QQmlListProperty<QObject> *property);

    int m_totalWindowCount;
    QHash<int, LipstickCompositorWindow *> m_mappedSurfaces;
    QHash<int, LipstickCompositorWindow *> m_windows;

    int m_nextWindowId;
    QList<WindowModel *> m_windowModels;

    bool m_homeActive;

    QVector<QObject *> m_data;
    QPointer<QQuickWindow> m_window;
    QScopedPointer<QWaylandOutput> m_output;
    QScopedPointer<QWaylandWlShell> m_wlShell;
    QScopedPointer<QWaylandXdgShellV5> m_xdgShell;
    QScopedPointer<AlienManager> m_alienManager;
    QQmlComponent *m_shaderEffect;
    QPointer<QWaylandSurface> m_fullscreenSurface;
    bool m_directRenderingActive;
    int m_topmostWindowId;
    int m_topmostWindowProcessId;
    Qt::ScreenOrientation m_topmostWindowOrientation;
    Qt::ScreenOrientation m_screenOrientation;
    Qt::ScreenOrientation m_sensorOrientation;
    QOrientationSensor* m_orientationSensor;
    QPointer<QMimeData> m_retainedSelection;
    MGConfItem *m_orientationLock;
    bool m_updatesEnabled;
    bool m_completed;
    int m_onUpdatesDisabledUnfocusedWindowId;
    LipstickKeymap *m_keymap;
    int m_fakeRepaintTimerId;

    QList<QueuedSetUpdatesEnabledCall> m_queuedSetUpdatesEnabledCalls;
    QMceNameOwner *m_mceNameOwner;
    bool m_fakeRepaintTriggered;
};

#endif // LIPSTICKCOMPOSITOR_H
