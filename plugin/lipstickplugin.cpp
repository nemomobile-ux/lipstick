
// This file is part of lipstick, a QML desktop library
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License version 2.1 as published by the Free Software Foundation
// and appearing in the file LICENSE.LGPL included in the packaging
// of this file.
//
// This code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// Copyright (c) 2012, Timur Kristóf <venemo@fedoraproject.org>
// Copyright (c) 2021 Chupligin Sergey (NeoChapay) <neochapay@gmail.com>

#include "lipstickplugin.h"

#include <QtQml>
#include <components/launcheritem.h>
#include <components/launcherwatchermodel.h>
#include <notifications/notificationpreviewpresenter.h>
#include <notifications/notificationfeedbackplayer.h>
#include <notifications/notificationlistmodel.h>
#include <notifications/lipsticknotification.h>
#include <volume/volumecontrol.h>
#include <usbmodeselector.h>
#include <shutdownscreen.h>
#include <bluetooth/bluetoothagent.h>
#include <bluetooth/bluetoothobexagent.h>
#include <compositor/lipstickcompositor.h>
#include <compositor/lipstickcompositorwindow.h>
#include <compositor/windowmodel.h>
#include <compositor/windowpixmapitem.h>
#include <lipstickapi.h>

static QObject *lipstickApi_callback(QQmlEngine *e, QJSEngine *)
{
    return new LipstickApi(e);
}

LipstickPlugin::LipstickPlugin(QObject *parent) :
    QQmlExtensionPlugin(parent)
{
}

void LipstickPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<LauncherModelType>(uri, 0, 1, "LauncherModel");
    qmlRegisterType<LauncherWatcherModel>(uri, 0, 1, "LauncherWatcherModel");
    qmlRegisterType<NotificationListModel>(uri, 0, 1, "NotificationListModel");
    qmlRegisterType<LipstickNotification>(uri, 0, 1, "Notification");
    qmlRegisterType<LauncherItem>(uri, 0, 1, "LauncherItem");
    qmlRegisterType<LauncherFolderModelType>(uri, 0, 1, "LauncherFolderModel");
    qmlRegisterType<LauncherFolderItem>(uri, 0, 1, "LauncherFolderItem");
    qmlRegisterType<VolumeControl>(uri, 0, 1, "VolumeControl");

    qmlRegisterUncreatableType<NotificationPreviewPresenter>(uri, 0, 1, "NotificationPreviewPresenter", "This type is initialized by HomeApplication");
    qmlRegisterUncreatableType<NotificationFeedbackPlayer>(uri, 0, 1, "NotificationFeedbackPlayer", "This type is initialized by HomeApplication");
    qmlRegisterUncreatableType<USBModeSelector>(uri, 0, 1, "USBModeSelector", "This type is initialized by HomeApplication");
    qmlRegisterUncreatableType<ShutdownScreen>(uri, 0, 1, "ShutdownScreen", "This type is initialized by HomeApplication");

    qmlRegisterType<LipstickCompositor>(uri, 0, 1, "Compositor");
    qmlRegisterUncreatableType<QWaylandSurface>(uri, 0, 1, "WaylandSurface", "This type is created by the compositor");
    qmlRegisterType<WindowModel>(uri, 0, 1, "WindowModel");
    qmlRegisterType<WindowPixmapItem>(uri, 0, 1, "WindowPixmapItem");
    qmlRegisterSingletonType<LipstickApi>(uri, 0, 1, "Lipstick", lipstickApi_callback);
    qmlRegisterUncreatableType<ScreenshotResult>(uri, 0, 1, "ScreenshotResult", "This type is initialized by LipstickApi");
    qmlRegisterUncreatableType<BluetoothAgent>(uri, 0, 1, "BluetoothAgent", "This type is created by lipstick");
    qmlRegisterUncreatableType<BluetoothObexAgent>(uri, 0, 1, "BluetoothObexAgent", "This type is created by lipstick");

    qmlRegisterRevision<QQuickWindow,1>(uri, 0, 1);
}
