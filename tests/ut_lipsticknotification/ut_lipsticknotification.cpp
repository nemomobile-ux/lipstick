/***************************************************************************
**
** Copyright (c) 2012 Jolla Ltd.
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
#include "qdbusargument_fake.h"
#include "ut_lipsticknotification.h"
#include "lipsticknotification.h"
#include "notificationmanager_stub.h"

void Ut_Notification::testGettersAndSetters()
{
    QString appName = "appName1";
    QString explicitAppName = "explicitAppName1";
    QString disambiguatedAppName = "appName1-2-3";
    uint id = 1;
    QString appIcon = "appIcon1";
    QString summary = "summary1";
    QString body = "body1";
    QString previewSummary = "previewSummary1";
    QString previewBody = "previewBody1";
    int urgency = 1;
    int itemCount = 1;
    int priority = 1;
    QString category = "category1";
    QStringList actions = QStringList() << "action1a" << "action1b";
    QDateTime timestamp = QDateTime::currentDateTime();
    QVariantHash hints;
    hints.insert(LipstickNotification::HINT_TIMESTAMP, timestamp);
    hints.insert(LipstickNotification::HINT_ITEM_COUNT, itemCount);
    hints.insert(LipstickNotification::HINT_PRIORITY, priority);
    hints.insert(LipstickNotification::HINT_PREVIEW_SUMMARY, previewSummary);
    hints.insert(LipstickNotification::HINT_PREVIEW_BODY, previewBody);
    hints.insert(LipstickNotification::HINT_URGENCY, urgency);
    hints.insert(LipstickNotification::HINT_CATEGORY, category);
    hints.insert("x-nemo.testing.custom-hint-value", M_PI);
    int expireTimeout = 1;

    // Ensure that the constructor puts things in place
    LipstickNotification notification(appName, explicitAppName, disambiguatedAppName, id, appIcon, summary, body, actions, hints, expireTimeout);
    QCOMPARE(notification.appName(), appName);
    QCOMPARE(notification.explicitAppName(), explicitAppName);
    QCOMPARE(notification.disambiguatedAppName(), disambiguatedAppName);
    QCOMPARE(notification.id(), id);
    QCOMPARE(notification.appIcon(), appIcon);
    QCOMPARE(notification.summary(), summary);
    QCOMPARE(notification.body(), body);
    QCOMPARE(notification.actions(), actions);
    QCOMPARE(notification.expireTimeout(), expireTimeout);
    QCOMPARE(notification.timestamp(), timestamp);
    QCOMPARE(notification.previewSummary(), previewSummary);
    QCOMPARE(notification.previewBody(), previewBody);
    QCOMPARE(notification.urgency(), urgency);
    QCOMPARE(notification.itemCount(), itemCount);
    QCOMPARE(notification.priority(), priority);
    QCOMPARE(notification.category(), category);
    QVERIFY(notification.hintValues().count() > 0);
    QVERIFY(notification.hintValues().contains("x-nemo.testing.custom-hint-value"));
    QVERIFY(!notification.hintValues().contains(LipstickNotification::HINT_CATEGORY));
    QCOMPARE(notification.hintValues().value("x-nemo.testing.custom-hint-value").toDouble(), M_PI);

    appName = "appName2";
    disambiguatedAppName = "appName2-2-3";
    appIcon = "appIcon2";
    summary = "summary2";
    body = "body2";
    previewSummary = "previewSummary2";
    previewBody = "previewBody2";
    urgency = 2;
    itemCount = 2;
    priority = 2;
    category = "category2";
    actions = QStringList() << "action2a" << "action2b" << "action2c";
    timestamp = QDateTime::currentDateTime();
    hints.insert(LipstickNotification::HINT_TIMESTAMP, timestamp);
    hints.insert(LipstickNotification::HINT_ITEM_COUNT, itemCount);
    hints.insert(LipstickNotification::HINT_PRIORITY, priority);
    hints.insert(LipstickNotification::HINT_PREVIEW_SUMMARY, previewSummary);
    hints.insert(LipstickNotification::HINT_PREVIEW_BODY, previewBody);
    hints.insert(LipstickNotification::HINT_URGENCY, urgency);
    hints.insert(LipstickNotification::HINT_CATEGORY, category);
    expireTimeout = 2;
    notification.setAppName(appName);
    notification.setDisambiguatedAppName(disambiguatedAppName);
    notification.setAppIcon(appIcon);
    notification.setSummary(summary);
    notification.setBody(body);
    notification.setActions(actions);
    notification.setHints(hints);
    notification.setExpireTimeout(expireTimeout);
    QCOMPARE(notification.appName(), appName);
    QCOMPARE(notification.disambiguatedAppName(), disambiguatedAppName);
    QCOMPARE(notification.appIcon(), appIcon);
    QCOMPARE(notification.summary(), summary);
    QCOMPARE(notification.body(), body);
    QCOMPARE(notification.actions(), actions);
    QCOMPARE(notification.expireTimeout(), expireTimeout);
    QCOMPARE(notification.timestamp(), timestamp);
    QCOMPARE(notification.previewSummary(), previewSummary);
    QCOMPARE(notification.previewBody(), previewBody);
    QCOMPARE(notification.urgency(), urgency);
    QCOMPARE(notification.itemCount(), itemCount);
    QCOMPARE(notification.priority(), priority);
    QCOMPARE(notification.category(), category);
}

void Ut_Notification::testIcon_data()
{
    QTest::addColumn<QString>("appIcon");
    QTest::addColumn<QString>("imagePath");

    QTest::newRow("No app_icon, no imagePath")
            << QString()
            << QString();
    QTest::newRow("No app_icon, imagePath")
            << QString()
            << QString("imagePath");
    QTest::newRow("app_icon, no imagePath")
            << QString("appIcon")
            << QString();
    QTest::newRow("app_icon, imagePath")
            << QString("appIcon")
            << QString("imagePath");
}

void Ut_Notification::testIcon()
{
    QFETCH(QString, appIcon);
    QFETCH(QString, imagePath);

    QVariantHash hints;
    if (!imagePath.isEmpty()) {
        hints.insert(LipstickNotification::HINT_IMAGE_PATH, imagePath);
    }

    LipstickNotification notification1(QString(), QString(), QString(), 0, appIcon, QString(), QString(), QStringList(), hints, 0);
    QCOMPARE(notification1.appIcon(), appIcon);
    QCOMPARE(notification1.hints().value(LipstickNotification::HINT_IMAGE_PATH).toString(), imagePath);

    LipstickNotification notification2(QString(), QString(), QString(), 0, QString(), QString(), QString(), QStringList(), QVariantHash(), 0);
    notification2.setHints(hints);
    QCOMPARE(notification2.hints().value(LipstickNotification::HINT_IMAGE_PATH).toString(), imagePath);

    LipstickNotification notification3(QString(), QString(), QString(), 0, QString(), QString(), QString(), QStringList(), hints, 0);
    QCOMPARE(notification3.hints().value(LipstickNotification::HINT_IMAGE_PATH).toString(), imagePath);
}

void Ut_Notification::testSignals()
{
    QVariantHash hints;
    LipstickNotification notification(QString(), QString(), QString(), 0, QString(), QString(), QString(), QStringList(), hints, 0);
    QSignalSpy summarySpy(&notification, SIGNAL(summaryChanged()));
    QSignalSpy bodySpy(&notification, SIGNAL(bodyChanged()));
    QSignalSpy timestampSpy(&notification, SIGNAL(timestampChanged()));
    QSignalSpy previewSummarySpy(&notification, SIGNAL(previewSummaryChanged()));
    QSignalSpy previewBodySpy(&notification, SIGNAL(previewBodyChanged()));
    QSignalSpy urgencySpy(&notification, SIGNAL(urgencyChanged()));

    notification.setSummary("summary");
    QCOMPARE(summarySpy.count(), 1);
    notification.setSummary("summary");
    QCOMPARE(summarySpy.count(), 1);

    notification.setBody("body");
    QCOMPARE(bodySpy.count(), 1);
    notification.setBody("body");
    QCOMPARE(bodySpy.count(), 1);

    hints.insert(LipstickNotification::HINT_TIMESTAMP, "2012-10-01 18:04:19");
    notification.setHints(hints);
    QCOMPARE(timestampSpy.count(), 1);

    notification.setHints(hints);
    QCOMPARE(timestampSpy.count(), 1);

    hints.insert(LipstickNotification::HINT_PREVIEW_SUMMARY, "previewSummary");
    notification.setHints(hints);
    QCOMPARE(previewSummarySpy.count(), 1);

    hints.insert(LipstickNotification::HINT_PREVIEW_BODY, "previewBody");
    notification.setHints(hints);
    QCOMPARE(previewSummarySpy.count(), 1);
    QCOMPARE(previewBodySpy.count(), 1);

    hints.insert(LipstickNotification::HINT_URGENCY, 2);
    notification.setHints(hints);
    QCOMPARE(previewBodySpy.count(), 1);
    QCOMPARE(urgencySpy.count(), 1);
}

void Ut_Notification::testSerialization()
{
    QString appName = "appName1";
    uint id = 1;
    QString appIcon = "appIcon1";
    QString summary = "summary1";
    QString body = "body1";
    QStringList actions = QStringList() << "action1a" << "action1b";
    QDateTime timestamp = QDateTime::currentDateTime();
    QVariantHash hints;
    hints.insert(LipstickNotification::HINT_TIMESTAMP, timestamp);
    int expireTimeout = 1;

    LipstickNotification n1(appName, appName, appName, id, appIcon, summary, body, actions, hints, expireTimeout);
    LipstickNotification n2;

    // Transfer a Notification from n1 to n2 by serializing it to a QDBusArgument and unserializing it
    QDBusArgument arg;
    arg << n1;
    arg >> n2;

    QCOMPARE(n2.appName(), n1.appName());
    QCOMPARE(n2.id(), n1.id());
    QCOMPARE(n2.appIcon(), n1.appIcon());
    QCOMPARE(n2.summary(), n1.summary());
    QCOMPARE(n2.body(), n1.body());
    QCOMPARE(n2.actions(), n1.actions());
    QCOMPARE(n2.expireTimeout(), n1.expireTimeout());
    QCOMPARE(n2.timestamp(), n1.timestamp());

    // Disambiguated app name is internal only
    QVERIFY(n2.disambiguatedAppName() != n1.appName());
}

QTEST_MAIN(Ut_Notification)
