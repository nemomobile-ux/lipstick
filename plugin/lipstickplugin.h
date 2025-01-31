
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

#ifndef LIPSTICKPLUGIN_H
#define LIPSTICKPLUGIN_H

#include <QQmlExtensionPlugin>
#include <QQmlParserStatus>
#include <QTranslator>
#include <QCoreApplication>

#include <components/launchermodel.h>
#include <components/launcherfoldermodel.h>

class Q_DECL_EXPORT LipstickPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.nemomobile.lipstick")

public:
    explicit LipstickPlugin(QObject *parent = 0);

    void initializeEngine(QQmlEngine *engine, const char *uri);
    void registerTypes(const char *uri);
    
};

class LauncherModelType : public LauncherModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
public:
    explicit LauncherModelType(QObject *parent = 0)
        : LauncherModel(DeferInitialization, parent)
    {
    }

    void classBegin() {}
    void componentComplete() { initialize(); }
};

class LauncherFolderModelType : public LauncherFolderModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
public:
    explicit LauncherFolderModelType(QObject *parent = 0)
        : LauncherFolderModel(DeferInitialization, parent)
    {
    }

    void classBegin() {}
    void componentComplete() { initialize(); }
};

class AppTranslator: public QTranslator
{
    Q_OBJECT
public:
    AppTranslator(QObject *parent)
        : QTranslator(parent)
    {
        qApp->installTranslator(this);
    }

    virtual ~AppTranslator()
    {
        qApp->removeTranslator(this);
    }
};

#endif // LIPSTICKPLUGIN_H
