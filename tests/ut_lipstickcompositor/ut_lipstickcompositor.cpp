/***************************************************************************
**
** Copyright (c) 2013 - 2023 Jolla Ltd.
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

#include <QtTest/QtTest>
#include <QClipboard>
#include <QQuickItem>
#include <QGuiApplication>

#include "homeapplication.h"
#include "lipstickcompositor.h"
#include "lipstickcompositorwindow.h"
#include "touchscreen/touchscreen.h"
#include "ut_lipstickcompositor.h"
#include "displaystate_stub.h"
#include "mdconfitem_stub.h"
#include "lipsticktest.h"

LipstickCompositorWindow *gTestWindow = nullptr;
LipstickCompositorWindow *gGhostWindow = nullptr;
TouchScreen *gTouchScreen = nullptr;

// The HomeApplication stub (../stubs/homeapplication.cpp) only implements the
// constructor and event(), so the remaining methods that dereference the
// uninitialized private members must be replaced in this translation unit
// (definitions in the executable preempt the ones from liblipstick-qt6.so).
HomeApplication::~HomeApplication()
{
}

HomeApplication *HomeApplication::instance()
{
    return qobject_cast<HomeApplication *>(qApp);
}

TouchScreen *HomeApplication::touchScreen() const
{
    return gTouchScreen;
}

void HomeApplication::setDisplayOff()
{
}

namespace {

// Drives the display state of the real TouchScreen through the stubbed
// DeviceState::DisplayStateMonitor, same mechanism as in ut_screenlock.
void setDisplayState(TouchScreen::DisplayState state)
{
    emit gDisplayStateMonitorStub->displayState->displayStateChanged(
        static_cast<DeviceState::DisplayStateMonitor::DisplayState>(state));
}

} // namespace

void Ut_LipstickCompositor::initTestCase()
{
    // Stub out MDConfItem — the orientationLock item then returns "dynamic"
    // and never touches dconf.
    gMDConfItemStub->stubSetReturnValue("value", QVariant("dynamic"));

    // Real TouchScreen with a stubbed DisplayStateMonitor backend.
    gTouchScreen = new TouchScreen;

    // Create the compositor.  Connections to MCE, timed, logind and the
    // orientation sensor fail gracefully in the test environment (they
    // only produce warnings).
    compositor = new LipstickCompositor();
    QVERIFY(LipstickCompositor::instance() == compositor);

    // A mapped window (id 1) and an unmapped "ghost" window (id 2),
    // registered directly in the compositor's bookkeeping tables.
    gTestWindow = new LipstickCompositorWindow(1, QString(), nullptr);
    compositor->m_windows.insert(1, gTestWindow);
    compositor->m_mappedSurfaces.insert(1, gTestWindow);
    gTestWindow->m_mapped = true;

    gGhostWindow = new LipstickCompositorWindow(2, QString(), nullptr);
    compositor->m_windows.insert(2, gGhostWindow);

    compositor->m_totalWindowCount = 2;
}

void Ut_LipstickCompositor::cleanupTestCase()
{
    // Deleting a LipstickCompositorWindow calls windowDestroyed() on the
    // live compositor, which removes it from the bookkeeping tables.
    delete gGhostWindow;
    gGhostWindow = nullptr;
    delete gTestWindow;
    gTestWindow = nullptr;

    delete compositor;
    compositor = nullptr;

    delete gTouchScreen;
    gTouchScreen = nullptr;

    gDisplayStateMonitorStub->stubReset();
    gMDConfItemStub->stubReset();
}

void Ut_LipstickCompositor::init()
{
    // Reset state before each test.
    compositor->m_homeActive = true;
    compositor->m_topmostWindowId = 0;
    compositor->m_topmostWindowProcessId = 0;
    compositor->m_topmostWindowPolicyApplicationId = QString();
    compositor->m_synthesizeBackEvent = true;
    compositor->m_topmostWindowOrientation = Qt::PrimaryOrientation;
    compositor->m_screenOrientation = Qt::PrimaryOrientation;
    compositor->m_sessionActive = false;

    // Restore the window tables that previous tests may have modified.
    compositor->m_windows.insert(1, gTestWindow);
    compositor->m_windows.insert(2, gGhostWindow);
    compositor->m_mappedSurfaces.insert(1, gTestWindow);
    compositor->m_mappedSurfaces.remove(2);
    gTestWindow->m_mapped = true;
    gGhostWindow->m_mapped = false;
    compositor->m_totalWindowCount = 2;

    setDisplayState(TouchScreen::DisplayOn);
}

void Ut_LipstickCompositor::cleanup()
{
}

void Ut_LipstickCompositor::testInstanceAndDefaults()
{
    QVERIFY(LipstickCompositor::instance() == compositor);

    QCOMPARE(compositor->windowCount(), 1); // only gTestWindow is mapped
    QCOMPARE(compositor->ghostWindowCount(), 1); // gGhostWindow is not mapped
    QCOMPARE(compositor->homeActive(), true);
    QCOMPARE(compositor->topmostWindowId(), 0);
    QCOMPARE(compositor->synthesizeBackEvent(), true);
}

void Ut_LipstickCompositor::testSetHomeActive()
{
    QCOMPARE(compositor->homeActive(), true);

    QSignalSpy spy(compositor, &LipstickCompositor::homeActiveChanged);
    compositor->setHomeActive(false);
    QCOMPARE(compositor->homeActive(), false);
    QCOMPARE(spy.count(), 1);

    // Setting the same value should not emit the signal.
    spy.clear();
    compositor->setHomeActive(false);
    QCOMPARE(spy.count(), 0);

    compositor->setHomeActive(true);
    QCOMPARE(compositor->homeActive(), true);
    QCOMPARE(spy.count(), 1);
}

void Ut_LipstickCompositor::testWindowCountAndGhostWindowCount()
{
    QCOMPARE(compositor->windowCount(), 1);
    QCOMPARE(compositor->ghostWindowCount(), 1);

    // Add a third mapped window.
    LipstickCompositorWindow *win3 = new LipstickCompositorWindow(3, QString(), nullptr);
    compositor->m_windows.insert(3, win3);
    compositor->m_mappedSurfaces.insert(3, win3);
    compositor->m_totalWindowCount = 3;
    win3->m_mapped = true;

    QCOMPARE(compositor->windowCount(), 2); // gTestWindow + win3
    QCOMPARE(compositor->ghostWindowCount(), 1); // gGhostWindow

    // Unmap gTestWindow.
    compositor->m_mappedSurfaces.remove(1);
    gTestWindow->m_mapped = false;

    QCOMPARE(compositor->windowCount(), 1); // only win3
    QCOMPARE(compositor->ghostWindowCount(), 2); // total 3 - mapped 1

    // Deleting win3 routes through windowDestroyed(): it is removed from
    // m_windows and unmapped.  m_totalWindowCount is only decremented by
    // windowObjectDestroyed(), which is not connected for manual windows.
    delete win3;
    QCOMPARE(compositor->windowCount(), 0);
    QCOMPARE(compositor->ghostWindowCount(), 3); // total 3 - mapped 0
    QVERIFY(compositor->windowForId(3) == nullptr);
}

void Ut_LipstickCompositor::testWindowForId()
{
    QObject *w = compositor->windowForId(1);
    QVERIFY(w != nullptr);
    QCOMPARE(qobject_cast<LipstickCompositorWindow *>(w), gTestWindow);

    // Non-existent window.
    QVERIFY(compositor->windowForId(999) == nullptr);

    // Ghost window exists in m_windows even though it is not mapped.
    QObject *gw = compositor->windowForId(2);
    QVERIFY(gw != nullptr);
    QCOMPARE(qobject_cast<LipstickCompositorWindow *>(gw), gGhostWindow);
}

void Ut_LipstickCompositor::testSurfaceForId()
{
    // surfaceForId delegates to window->surface().  Our test windows are
    // constructed without a surface, so it returns 0 for them, as well as
    // for unknown ids.
    QVERIFY(compositor->surfaceForId(1) == nullptr);
    QVERIFY(compositor->surfaceForId(999) == nullptr);
}

void Ut_LipstickCompositor::testSetTopmostWindowId()
{
    QCOMPARE(compositor->topmostWindowId(), 0);

    QSignalSpy spy(compositor, &LipstickCompositor::topmostWindowIdChanged);
    compositor->setTopmostWindowId(1);
    QCOMPARE(compositor->topmostWindowId(), 1);
    QCOMPARE(spy.count(), 1);

    // Setting the same value should not emit.
    spy.clear();
    compositor->setTopmostWindowId(1);
    QCOMPARE(spy.count(), 0);
}

void Ut_LipstickCompositor::testTopmostWindowProcessAndPolicyApplicationId()
{
    compositor->m_topmostWindowProcessId = 0;
    compositor->m_topmostWindowPolicyApplicationId = QString();

    QSignalSpy pidSpy(compositor, &LipstickCompositor::privateTopmostWindowProcessIdChanged);
    QSignalSpy appSpy(compositor, &LipstickCompositor::privateTopmostWindowPolicyApplicationIdChanged);

    // gTestWindow has processId 0 and an empty policyApplicationId.
    // The pid does not change (0 -> 0), so no pid signal; the policy
    // application id becomes "none".
    compositor->setTopmostWindowId(1);
    QCOMPARE(compositor->privateTopmostWindowProcessId(), 0);
    QCOMPARE(compositor->privateTopmostWindowPolicyApplicationId(), QString("none"));
    QCOMPARE(pidSpy.count(), 0);
    QCOMPARE(appSpy.count(), 1);

    // A non-existent window yields pid -1; policy id stays "none".
    compositor->setTopmostWindowId(999);
    QCOMPARE(compositor->privateTopmostWindowProcessId(), -1);
    QCOMPARE(pidSpy.count(), 1);
    QCOMPARE(appSpy.count(), 1);
}

void Ut_LipstickCompositor::testSetTopmostWindowOrientation()
{
    QCOMPARE(compositor->topmostWindowOrientation(), Qt::PrimaryOrientation);

    QSignalSpy spy(compositor, &LipstickCompositor::topmostWindowOrientationChanged);
    compositor->setTopmostWindowOrientation(Qt::LandscapeOrientation);
    QCOMPARE(compositor->topmostWindowOrientation(), Qt::LandscapeOrientation);
    QCOMPARE(spy.count(), 1);

    // Setting the same value should not emit.
    spy.clear();
    compositor->setTopmostWindowOrientation(Qt::LandscapeOrientation);
    QCOMPARE(spy.count(), 0);
}

void Ut_LipstickCompositor::testSetScreenOrientation()
{
    QCOMPARE(compositor->screenOrientation(), Qt::PrimaryOrientation);

    QSignalSpy spy(compositor, &LipstickCompositor::screenOrientationChanged);
    compositor->setScreenOrientation(Qt::LandscapeOrientation);
    QCOMPARE(compositor->screenOrientation(), Qt::LandscapeOrientation);
    QCOMPARE(spy.count(), 1);

    // Setting the same value should not emit.
    spy.clear();
    compositor->setScreenOrientation(Qt::LandscapeOrientation);
    QCOMPARE(spy.count(), 0);
}

void Ut_LipstickCompositor::testSetSynthesizeBackEvent()
{
    QCOMPARE(compositor->synthesizeBackEvent(), true);

    QSignalSpy spy(compositor, &LipstickCompositor::synthesizeBackEventChanged);
    compositor->setSynthesizeBackEvent(false);
    QCOMPARE(compositor->synthesizeBackEvent(), false);
    QCOMPARE(spy.count(), 1);

    // Setting the same value should not emit.
    spy.clear();
    compositor->setSynthesizeBackEvent(false);
    QCOMPARE(spy.count(), 0);
}

void Ut_LipstickCompositor::testReactOnDisplayStateChanges()
{
    QSignalSpy offSpy(compositor, &LipstickCompositor::displayOff);
    QSignalSpy onSpy(compositor, &LipstickCompositor::displayOn);
    QSignalSpy dimmedSpy(compositor, &LipstickCompositor::displayDimmedChanged);

    // On -> Dimmed: display stays on, but dimmed state changes.
    compositor->reactOnDisplayStateChanges(TouchScreen::DisplayOn,
                                           TouchScreen::DisplayDimmed);
    QCOMPARE(dimmedSpy.count(), 1);
    QCOMPARE(offSpy.count(), 0);
    QCOMPARE(onSpy.count(), 0);

    // Dimmed -> Off: display goes off and is no longer dimmed.
    compositor->reactOnDisplayStateChanges(TouchScreen::DisplayDimmed,
                                           TouchScreen::DisplayOff);
    QCOMPARE(offSpy.count(), 1);
    QCOMPARE(onSpy.count(), 0);
    QCOMPARE(dimmedSpy.count(), 2);

    // Off -> On: display comes on.
    compositor->reactOnDisplayStateChanges(TouchScreen::DisplayOff,
                                           TouchScreen::DisplayOn);
    QCOMPARE(onSpy.count(), 1);

    // On -> On: nothing is emitted.
    compositor->reactOnDisplayStateChanges(TouchScreen::DisplayOn,
                                           TouchScreen::DisplayOn);
    QCOMPARE(offSpy.count(), 1);
    QCOMPARE(onSpy.count(), 1);
    QCOMPARE(dimmedSpy.count(), 2);
}

void Ut_LipstickCompositor::testDisplayDimmedAndDisplayAmbient()
{
    setDisplayState(TouchScreen::DisplayOn);
    QVERIFY(!compositor->displayDimmed());
    QVERIFY(!compositor->displayAmbient());

    setDisplayState(TouchScreen::DisplayDimmed);
    QVERIFY(compositor->displayDimmed());
    QVERIFY(compositor->displayAmbient());

    setDisplayState(TouchScreen::DisplayOff);
    QVERIFY(!compositor->displayDimmed());
    QVERIFY(!compositor->displayAmbient());
}

void Ut_LipstickCompositor::testClipboard()
{
    QObject *cb = compositor->clipboard();
    QVERIFY(cb != nullptr);
    QCOMPARE(cb, static_cast<QObject *>(QGuiApplication::clipboard()));
}

void Ut_LipstickCompositor::testCloseClientForWindowIdWithoutSurface()
{
    // Unknown window id — no crash.
    compositor->closeClientForWindowId(999);

    // Known window without a surface — no crash.
    compositor->closeClientForWindowId(1);
}

void Ut_LipstickCompositor::testRespondSupportCheckForUnknownRequest()
{
    // respondSupportCheck for a request id that was never queued is a no-op.
    compositor->respondSupportCheck(9999, true);
}

void Ut_LipstickCompositor::testSetUpdatesEnabledNowCompletes()
{
    // topmostWindowId is 0, so windowForId(0) returns nullptr — disabling
    // updates must not crash even without a focused window.
    compositor->setUpdatesEnabledNow(false);

    // Enabling updates again shows the window and flips completed() to true.
    compositor->setUpdatesEnabledNow(true);
    QVERIFY(compositor->completed());
}

void Ut_LipstickCompositor::testEventWithoutBackSynthesis()
{
    // With back synthesis disabled a right button event falls through to
    // QObject::event(), which ignores mouse events.
    compositor->setSynthesizeBackEvent(false);

    QMouseEvent mousePress(QEvent::MouseButtonPress, QPointF(0, 0),
                           QPointF(0, 0), Qt::RightButton,
                           Qt::RightButton, Qt::NoModifier);
    QVERIFY(!compositor->event(&mousePress));
}

LIPSTICK_TEST_MAIN(Ut_LipstickCompositor)
