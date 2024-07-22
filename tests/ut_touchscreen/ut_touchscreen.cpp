/***************************************************************************
**
** Copyright (c) 2016 - 2020 Jolla Ltd.
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
#include <QMouseEvent>

#include "ut_touchscreen.h"
#include "touchscreen.h"
#include "homeapplication.h"
#include "displaystate_stub.h"
#include "lipsticktest.h"

#include <mce/mode-names.h>

TouchScreen *gTouchScreen = 0;

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

void Ut_TouchScreen::init()
{
    gTouchScreen = new TouchScreen;
}

void Ut_TouchScreen::cleanup()
{
    delete gTouchScreen;
}

void Ut_TouchScreen::initTestCase()
{
}

void Ut_TouchScreen::cleanupTestCase()
{
}

void Ut_TouchScreen::testEnabled()
{
    fakeDisplayOnAndReady();
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(), QPointF(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);

    TouchScreen *touchScreen = HomeApplication::instance()->touchScreen();
    touchScreen->setEnabled(false);
    QCOMPARE(touchScreen->eventFilter(0, &event), true);

    touchScreen->setEnabled(true);
    QCOMPARE(touchScreen->eventFilter(0, &event), false);
}

void Ut_TouchScreen::testTouchBlocking()
{
    HomeApplication::instance()->touchScreen()->setEnabled(true);

    TouchScreen *touchScreen = HomeApplication::instance()->touchScreen();
    updateDisplayState(DeviceState::DisplayStateMonitor::Unknown, DeviceState::DisplayStateMonitor::Off);
    touchScreen->inputPolicyChanged(MCE_INPUT_POLICY_DISABLED);
    QVERIFY(touchScreen->touchBlocked());

    QMouseEvent event(QEvent::MouseButtonPress, QPointF(), QPointF(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCOMPARE(touchScreen->eventFilter(0, &event), true);

    QMouseEvent event1(QEvent::MouseMove, QPointF(), QPointF(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCOMPARE(touchScreen->eventFilter(0, &event1), true);

    QMouseEvent event2(QEvent::MouseButtonRelease, QPointF(), QPointF(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCOMPARE(touchScreen->eventFilter(0, &event2), true);

    QMouseEvent event3(QEvent::MouseButtonDblClick, QPointF(), QPointF(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCOMPARE(touchScreen->eventFilter(0, &event3), true);

    QTouchEvent touch(QEvent::TouchBegin);
    QCOMPARE(touchScreen->eventFilter(0, &touch), true);

    QTouchEvent touch1(QEvent::TouchUpdate);
    QCOMPARE(touchScreen->eventFilter(0, &touch1), true);

    QTouchEvent touch2(QEvent::TouchEnd);
    QCOMPARE(touchScreen->eventFilter(0, &touch2), true);

    // Do not filter TouchCancel.
    QTouchEvent touch3(QEvent::TouchCancel);
    QCOMPARE(touchScreen->eventFilter(0, &touch3), false);

    updateDisplayState(DeviceState::DisplayStateMonitor::Off, DeviceState::DisplayStateMonitor::On);
    QVERIFY(touchScreen->touchBlocked());
    touchScreen->inputPolicyChanged(MCE_INPUT_POLICY_ENABLED);
    QSignalSpy touchBlockingSpy(touchScreen, SIGNAL(touchBlockedChanged()));
    touchBlockingSpy.wait();
    QVERIFY(!touchScreen->touchBlocked());

    QMouseEvent event4(QEvent::MouseButtonRelease, QPointF(), QPointF(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCOMPARE(touchScreen->eventFilter(0, &event4), true);

    QMouseEvent event5(QEvent::MouseButtonDblClick,  QPointF(), QPointF(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCOMPARE(touchScreen->eventFilter(0, &event5), true);

    QTouchEvent touch4(QEvent::TouchUpdate);
    QCOMPARE(touchScreen->eventFilter(0, &touch4), true);

    QTouchEvent touch5(QEvent::TouchEnd);
    QCOMPARE(touchScreen->eventFilter(0, &touch5), true);

    // New touch sequence starts after turning display on.
    QTouchEvent touch6(QEvent::TouchBegin);
    QCOMPARE(touchScreen->eventFilter(0, &touch6), false);

    QTouchEvent touch7(QEvent::TouchUpdate);
    QCOMPARE(touchScreen->eventFilter(0, &touch7), false);

    QTouchEvent touch8(QEvent::TouchEnd);
    QCOMPARE(touchScreen->eventFilter(0, &touch8), false);

    QMouseEvent event6(QEvent::MouseButtonPress, QPointF(), QPointF(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCOMPARE(touchScreen->eventFilter(0, &event6), false);

    QMouseEvent event7(QEvent::MouseMove, QPointF(), QPointF(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCOMPARE(touchScreen->eventFilter(0, &event7), false);

    QMouseEvent event8(QEvent::MouseButtonRelease, QPointF(), QPointF(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCOMPARE(touchScreen->eventFilter(0, &event8), false);

    // MouseButtonPress stops event eater
    touchScreen->inputPolicyChanged(MCE_INPUT_POLICY_DISABLED);
    touchBlockingSpy.wait();
    QVERIFY(touchScreen->touchBlocked());
    touchScreen->inputPolicyChanged(MCE_INPUT_POLICY_ENABLED);
    touchBlockingSpy.wait();
    QVERIFY(!touchScreen->touchBlocked());

    QTouchEvent touch9(QEvent::TouchUpdate);
    QCOMPARE(touchScreen->eventFilter(0, &touch9), true);

    QMouseEvent event9(QEvent::MouseButtonPress, QPointF(), QPointF(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCOMPARE(touchScreen->eventFilter(0, &event9), false);

    QTouchEvent touch10(QEvent::TouchUpdate);
    QCOMPARE(touchScreen->eventFilter(0, &touch10), false);

    // MouseMove stops event eater
    touchScreen->inputPolicyChanged(MCE_INPUT_POLICY_DISABLED);
    touchBlockingSpy.wait();
    QVERIFY(touchScreen->touchBlocked());
    touchScreen->inputPolicyChanged(MCE_INPUT_POLICY_ENABLED);
    touchBlockingSpy.wait();
    QVERIFY(!touchScreen->touchBlocked());

    QTouchEvent touch11(QEvent::TouchUpdate);
    QCOMPARE(touchScreen->eventFilter(0, &touch11), true);

    QMouseEvent event10(QEvent::MouseMove, QPointF(), QPointF(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCOMPARE(touchScreen->eventFilter(0, &event10), false);

    QTouchEvent touch12(QEvent::TouchUpdate);
    QCOMPARE(touchScreen->eventFilter(0, &touch12), false);
}

void Ut_TouchScreen::updateDisplayState(DeviceState::DisplayStateMonitor::DisplayState oldState, DeviceState::DisplayStateMonitor::DisplayState newState)
{
    emit gDisplayStateMonitorStub->displayState->displayStateChanged(oldState);
    emit gDisplayStateMonitorStub->displayState->displayStateChanged(newState);
}

void Ut_TouchScreen::fakeDisplayOnAndReady()
{
    // Fake display state change.
    TouchScreen *touchScreen = HomeApplication::instance()->touchScreen();
    updateDisplayState(DeviceState::DisplayStateMonitor::Off, DeviceState::DisplayStateMonitor::On);
    touchScreen->inputPolicyChanged(MCE_INPUT_POLICY_ENABLED);
    QSignalSpy touchBlockingSpy(touchScreen, SIGNAL(touchBlockedChanged()));
    touchBlockingSpy.wait();

    QVERIFY(!touchScreen->touchBlocked());
    touchScreen->setEnabled(true);
    QTouchEvent touch(QEvent::TouchBegin);
    QCOMPARE(touchScreen->eventFilter(0, &touch), false);
}

LIPSTICK_TEST_MAIN(Ut_TouchScreen)
