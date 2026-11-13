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

#ifndef UT_LIPSTICKCOMPOSITOR_H
#define UT_LIPSTICKCOMPOSITOR_H

#include <QtTest/QtTest>

class LipstickCompositor;
class LipstickCompositorWindow;
class TouchScreen;

// Global test windows — visible to all test methods in this TU.
extern LipstickCompositorWindow *gTestWindow;
extern LipstickCompositorWindow *gGhostWindow;

class Ut_LipstickCompositor : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testInstanceAndDefaults();
    void testSetHomeActive();
    void testWindowCountAndGhostWindowCount();
    void testWindowForId();
    void testSurfaceForId();
    void testSetTopmostWindowId();
    void testTopmostWindowProcessAndPolicyApplicationId();
    void testSetTopmostWindowOrientation();
    void testSetScreenOrientation();
    void testSetSynthesizeBackEvent();
    void testReactOnDisplayStateChanges();
    void testDisplayDimmedAndDisplayAmbient();
    void testClipboard();
    void testCloseClientForWindowIdWithoutSurface();
    void testRespondSupportCheckForUnknownRequest();
    void testSetUpdatesEnabledNowCompletes();
    void testEventWithoutBackSynthesis();

private:
    LipstickCompositor *compositor;
};

#endif
