/***************************************************************************
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (c) 2012 - 2020 Jolla Ltd.
** Copyright (c) 2020 Open Mobile Platform LLC.
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
#include <QDBusAbstractInterface>
#include <QMouseEvent>

#define TEST_SERVICE   QString("com.nokia.mcetest")
#define TEST_PATH      QString("/com/nokia/mcetest")
#define TEST_INTERFACE QString("com.nokia.mcetest")
#define TEST_METHOD    QString("testmethod")

#include "ut_screenlock.h"
#include "screenlock.h"
#include "touchscreen/touchscreen.h"
#include "homeapplication.h"
#include "closeeventeater_stub.h"
#include "displaystate_stub.h"
#include "lipsticktest.h"

TouchScreen *gTouchScreen = 0;

void QTimer::singleShot(int, const QObject *receiver, const char *member)
{
    // The "member" string is of form "1member()", so remove the trailing 1 and the ()
    int memberLength = strlen(member) - 3;
    char modifiedMember[memberLength + 1];
    strncpy(modifiedMember, member + 1, memberLength);
    modifiedMember[memberLength] = 0;
    QMetaObject::invokeMethod(const_cast<QObject *>(receiver), modifiedMember, Qt::DirectConnection);
}

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

void Ut_ScreenLock::init()
{
    gTouchScreen = new TouchScreen;
    screenLock = new ScreenLock(gTouchScreen);
}

void Ut_ScreenLock::cleanup()
{
    delete screenLock;
}

void Ut_ScreenLock::initTestCase()
{
}

void Ut_ScreenLock::cleanupTestCase()
{
}

void Ut_ScreenLock::testToggleScreenLockUI()
{
    QSignalSpy spy(screenLock, SIGNAL(screenLockedChanged(bool)));

    // When the lock is toggled on, make sure the lock UI is shown
    screenLock->setScreenLocked(true);

    // Locked state should be set
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.last().at(0).toBool(), true);
    QCOMPARE(screenLock->isScreenLocked(), true);

    // When the lock is toggled off, make sure the lock UI is hidden
    screenLock->setScreenLocked(false);

    // Locked state should not be set
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.last().at(0).toBool(), false);
    QCOMPARE(screenLock->isScreenLocked(), false);
}

void Ut_ScreenLock::testToggleEventEater()
{
    fakeDisplayOnAndReady();
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(), Qt::NoButton, 0, 0);

    // Make sure the screen locking signals are sent and the eater UI is shown/hidden
    screenLock->setEventEaterEnabled(true);
    QCOMPARE(gTouchScreen->eventFilter(0, &event), true);

    screenLock->setEventEaterEnabled(false);
    QCOMPARE(gTouchScreen->eventFilter(0, &event), false);
}

void Ut_ScreenLock::testUnlockScreenWhenLocked()
{
    screenLock->tklock_open(TEST_SERVICE, TEST_PATH, TEST_INTERFACE, TEST_METHOD, ScreenLock::TkLockModeNone, false, false);
    screenLock->setScreenLocked(true);
    screenLock->setEventEaterEnabled(true);
    screenLock->unlockScreen();

    QString member(screenLock->m_callbackMethod.member());
    QString path(screenLock->m_callbackMethod.path());
    QString service(screenLock->m_callbackMethod.service());
    QString interface(screenLock->m_callbackMethod.interface());
    QList<QVariant> arguments(screenLock->m_callbackMethod.arguments());
    bool modeOk = false;
    int mode(arguments.value(0).toInt(&modeOk));
    if (!modeOk) {
        qWarning("mode arg not present");
        mode = -1;
    }

    QCOMPARE(member, TEST_METHOD);
    QCOMPARE(path, TEST_PATH);
    QCOMPARE(service, TEST_SERVICE);
    QCOMPARE(interface, TEST_INTERFACE);
    QCOMPARE(mode, int(ScreenLock::TkLockUnlock));
}

void Ut_ScreenLock::testUnlockScreenWhenNotLocked()
{
    screenLock->unlockScreen();
}

void Ut_ScreenLock::testTkLockOpen_data()
{
    QTest::addColumn<int>("mode");
    QTest::addColumn<bool>("mainWindowModified");
    QTest::addColumn<bool>("screenLockedChanged");
    QTest::addColumn<bool>("eventEaterWindowVisibilityModified");
    QTest::addColumn<bool>("eventEaterWindowVisible");

    QTest::newRow("TkLockModeNone") << (int)ScreenLock::TkLockModeNone << false << false << false << false;
    QTest::newRow("TkLockModeEnable") << (int)ScreenLock::TkLockModeEnable << true << true << true << false;
    QTest::newRow("TkLockModeHelp") << (int)ScreenLock::TkLockModeHelp << false << false << false << false;
    QTest::newRow("TkLockModeSelect") << (int)ScreenLock::TkLockModeSelect << false << false << false << false;
    QTest::newRow("TkLockModeOneInput") << (int)ScreenLock::TkLockModeOneInput << false << false << true << true;
    QTest::newRow("TkLockEnableVisual") << (int)ScreenLock::TkLockEnableVisual << true << true << true << false;
    QTest::newRow("TkLockEnableLowPowerMode") << (int)ScreenLock::TkLockEnableLowPowerMode << true << true << true << false;
    QTest::newRow("TkLockRealBlankMode") << (int)ScreenLock::TkLockRealBlankMode << true << true << true << false;
}

void Ut_ScreenLock::testTkLockOpen()
{
    QFETCH(int, mode);
    QFETCH(bool, mainWindowModified);
    QFETCH(bool, screenLockedChanged);
    QFETCH(bool, eventEaterWindowVisibilityModified);
    QFETCH(bool, eventEaterWindowVisible);

    fakeDisplayOnAndReady();
    // Make sure the event eater is visible so that it will be hidden if necessary
    screenLock->showEventEater();

    // Modify the state
    QSignalSpy spy(screenLock, SIGNAL(screenLockedChanged(bool)));
    int reply = screenLock->tklock_open(TEST_SERVICE, TEST_PATH, TEST_INTERFACE, TEST_METHOD, mode, false, false);
    QCOMPARE(reply, (int)ScreenLock::TkLockReplyOk);

    // Check that main window title and stacking layer were only changed if needed (and to the correct state)
    if (mainWindowModified) {
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.last().at(0).toBool(), screenLockedChanged);
    }

    if (eventEaterWindowVisibilityModified) {
        QMouseEvent event(QEvent::MouseButtonPress, QPointF(), Qt::NoButton, 0, 0);
        TouchScreen *touchScreen = HomeApplication::instance()->touchScreen();
        QCOMPARE(touchScreen->eventFilter(0, &event), eventEaterWindowVisible);
    }
}

void Ut_ScreenLock::testTkLockClose()
{
    // Show the screen lock window and the event eater
    fakeDisplayOnAndReady();
    screenLock->showScreenLock();
    screenLock->showEventEater();

    // Modify the state
    QSignalSpy spy(screenLock, SIGNAL(screenLockedChanged(bool)));
    int reply = screenLock->tklock_close(false);
    QCOMPARE(reply, (int)ScreenLock::TkLockReplyOk);

    // The screen should no longer be locked
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.last().at(0).toBool(), false);

    // Events should still be eaten
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(), Qt::NoButton, 0, 0);
    TouchScreen *touchScreen = HomeApplication::instance()->touchScreen();
    QCOMPARE(touchScreen->eventFilter(0, &event), true);
}

void Ut_ScreenLock::updateDisplayState(DeviceState::DisplayStateMonitor::DisplayState oldState, DeviceState::DisplayStateMonitor::DisplayState newState)
{
    emit gDisplayStateMonitorStub->displayState->displayStateChanged(oldState);
    emit gDisplayStateMonitorStub->displayState->displayStateChanged(newState);
}

void Ut_ScreenLock::fakeDisplayOnAndReady()
{
    // Fake display state change.
    TouchScreen *touchScreen = HomeApplication::instance()->touchScreen();
    updateDisplayState(DeviceState::DisplayStateMonitor::Off, DeviceState::DisplayStateMonitor::On);
    QSignalSpy touchBlockingSpy(screenLock, SIGNAL(touchBlockedChanged()));
    touchBlockingSpy.wait();

    QVERIFY(!screenLock->touchBlocked());
    touchScreen->setEnabled(true);
    QTouchEvent touch(QEvent::TouchBegin);
    QCOMPARE(touchScreen->eventFilter(0, &touch), false);
}

LIPSTICK_TEST_MAIN(Ut_ScreenLock)
