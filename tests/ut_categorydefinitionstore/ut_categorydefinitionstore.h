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
#ifndef UT_CATEGORYDEFINITIONSTORE_H
#define UT_CATEGORYDEFINITIONSTORE_H

#include <QObject>
#include <displaystate.h>

class CategoryDefinitionStore;

class Ut_CategoryDefinitionStore : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void testCategoryDefinitionSettingsValues();
    void testCategoryDefinitionStoreMaxFileSizeHandling();
    void testCategoryDefinitionUninstalling();

private:
    void updateDisplayState(DeviceState::DisplayStateMonitor::DisplayState oldState, DeviceState::DisplayStateMonitor::DisplayState newState);
    void fakeDisplayOnAndReady();

signals:
    void directoryChanged(const QString &path);
};

#endif
