/***************************************************************************
**
** Copyright (C) 2012 Jolla Ltd.
** Contact: Robin Burchell <robin.burchell@jollamobile.com>
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

#include <NgfClient>
#include <QWaylandSurface>
#include "lipstickcompositor.h"
#include "lipstickcompositorwindow.h"
#include "notificationmanager.h"
#include "notificationfeedbackplayer.h"

namespace {

enum PreviewMode {
    AllNotificationsEnabled = 0,
    ApplicationNotificationsDisabled,
    SystemNotificationsDisabled,
    AllNotificationsDisabled
};

}

NotificationFeedbackPlayer::NotificationFeedbackPlayer(QObject *parent) :
    QObject(parent),
    m_ngfClient(new Ngf::Client(this)),
    m_minimumPriority(0)
{
    connect(NotificationManager::instance(), SIGNAL(notificationRemoved(uint)), this, SLOT(removeNotification(uint)));

    QTimer::singleShot(0, this, SLOT(init()));
}

void NotificationFeedbackPlayer::init()
{
    m_ngfClient->connect();
}

void NotificationFeedbackPlayer::addNotification(uint id)
{
    LipstickNotification *notification = NotificationManager::instance()->notification(id);

    if (notification != 0 && isEnabled(notification)) {
        // Stop feedback previously generated by this notification, if current
        QMultiHash<LipstickNotification *, uint>::iterator it = m_idToEventId.find(notification);
        while (it != m_idToEventId.end() && it.key() == notification) {
            m_ngfClient->stop(it.value());
            it = m_idToEventId.erase(it);
        }

        // Play the feedback related to the notification if any
        const QString feedback = notification->hints().value(NotificationManager::HINT_FEEDBACK).toString();
        const QStringList feedbackItems = feedback.split(QStringLiteral(","), QString::SkipEmptyParts);
        if (!feedbackItems.isEmpty()) {
            QMap<QString, QVariant> properties;
            if (notification->hints().value(NotificationManager::HINT_LED_DISABLED_WITHOUT_BODY_AND_SUMMARY, true).toBool() &&
                    notification->body().isEmpty() &&
                    notification->summary().isEmpty()) {
                properties.insert("media.leds", false);
            }
            if (notification->hints().value(NotificationManager::HINT_SUPPRESS_SOUND, false).toBool()) {
                properties.insert("media.audio", false);
            }

            if (!properties.isEmpty()) {
                // Add any unspecified properties as true, or they are treated as false (when properties is non-empty)
                if (!properties.contains("media.leds")) {
                    properties.insert("media.leds", true);
                }
                if (!properties.contains("media.audio")) {
                    properties.insert("media.audio", true);
                }
                properties.insert("media.vibra", true);
                properties.insert("media.backlight", true);
            }

            foreach (const QString &item, feedbackItems) {
                m_ngfClient->stop(item);
                m_idToEventId.insert(notification, m_ngfClient->play(item, properties));
            }
        }
    }
}

void NotificationFeedbackPlayer::removeNotification(uint id)
{
    LipstickNotification *notification = NotificationManager::instance()->notification(id);

    if (notification != 0) {
        // Stop the feedback related to the notification, if any
        QMultiHash<LipstickNotification *, uint>::iterator it = m_idToEventId.find(notification);
        while (it != m_idToEventId.end() && it.key() == notification) {
            m_ngfClient->stop(it.value());
            it = m_idToEventId.erase(it);
        }
    }
}

bool NotificationFeedbackPlayer::isEnabled(LipstickNotification *notification)
{
    if (notification->hidden() || notification->restored())
        return false;

    uint mode = AllNotificationsEnabled;
    LipstickCompositorWindow *win = LipstickCompositor::instance()->m_windows.value(LipstickCompositor::instance()->topmostWindowId());
    if (win != 0) {
        mode = win->windowProperties().value("NOTIFICATION_PREVIEWS_DISABLED", uint(AllNotificationsEnabled)).toUInt();
    }

    int urgency = notification->urgency();
    int priority = notification->priority();
    int notificationIsCritical = urgency >= 2 || notification->hints().value(NotificationManager::HINT_DISPLAY_ON).toBool();

    return !(urgency < 2 && priority < m_minimumPriority) &&
           (mode == AllNotificationsEnabled ||
            (mode == ApplicationNotificationsDisabled && notificationIsCritical) ||
            (mode == SystemNotificationsDisabled && urgency < 2));
}

int NotificationFeedbackPlayer::minimumPriority() const
{
    return m_minimumPriority;
}

void NotificationFeedbackPlayer::setMinimumPriority(int minimumPriority)
{
    m_minimumPriority = minimumPriority;

    emit minimumPriorityChanged();
}
