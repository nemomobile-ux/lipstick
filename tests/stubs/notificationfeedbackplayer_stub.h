/***************************************************************************
**
** Copyright (c) 2012-2014 Jolla Ltd.
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
#ifndef NOTIFICATIONFEEDBACKPLAYER_STUB
#define NOTIFICATIONFEEDBACKPLAYER_STUB

#include "notificationfeedbackplayer.h"
#include <stubbase.h>


// 1. DECLARE STUB
// FIXME - stubgen is not yet finished
class NotificationFeedbackPlayerStub : public StubBase
{
public:
    virtual void NotificationFeedbackPlayerConstructor(QObject *parent);
    virtual void init();
    virtual bool addNotification(uint id);
    virtual void removeNotification(uint id);
};

// 2. IMPLEMENT STUB
void NotificationFeedbackPlayerStub::NotificationFeedbackPlayerConstructor(QObject *parent)
{
    Q_UNUSED(parent);
}

void NotificationFeedbackPlayerStub::init()
{
    stubMethodEntered("init");
}

bool NotificationFeedbackPlayerStub::addNotification(uint id)
{
    QList<ParameterBase *> params;
    params.append(new Parameter<uint >(id));
    stubMethodEntered("addNotification", params);
    return stubReturnValue<bool>("addNotification");
}

void NotificationFeedbackPlayerStub::removeNotification(uint id)
{
    QList<ParameterBase *> params;
    params.append(new Parameter<uint >(id));
    stubMethodEntered("removeNotification", params);
}


// 3. CREATE A STUB INSTANCE
NotificationFeedbackPlayerStub gDefaultNotificationFeedbackPlayerStub;
NotificationFeedbackPlayerStub *gNotificationFeedbackPlayerStub = &gDefaultNotificationFeedbackPlayerStub;


// 4. CREATE A PROXY WHICH CALLS THE STUB
NotificationFeedbackPlayer::NotificationFeedbackPlayer(QObject *parent)
    : m_doNotDisturbSetting("/lipstick/do_not_disturb")
{
    gNotificationFeedbackPlayerStub->NotificationFeedbackPlayerConstructor(parent);
}

void NotificationFeedbackPlayer::init()
{
    gNotificationFeedbackPlayerStub->init();
}

bool NotificationFeedbackPlayer::addNotification(uint id)
{
    return gNotificationFeedbackPlayerStub->addNotification(id);
}

void NotificationFeedbackPlayer::removeNotification(uint id)
{
    gNotificationFeedbackPlayerStub->removeNotification(id);
}

bool NotificationFeedbackPlayer::doNotDisturbMode() const
{
    return false;
}

#endif
