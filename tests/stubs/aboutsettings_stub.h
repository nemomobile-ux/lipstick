/***************************************************************************
**
** Copyright (c) 2019 Open Mobile Platform LLC.
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

#ifndef ABOUTSETTINGS_H_STUB
#define ABOUTSETTINGS_H_STUB

#include "aboutsettings.h"
#include <stubbase.h>

static const auto WlanMacAddress = QStringLiteral("wlanMacAddress");
static const auto Serial = QStringLiteral("serial");
static const auto LocalizedOperatingSystemName = QStringLiteral("localizedOperatingSystemName");
static const auto OperatingSystemName = QStringLiteral("operatingSystemName");
static const auto LocalizedSoftwareVersion = QStringLiteral("localizedSoftwareVersion");
static const auto BaseOperatingSystemName = QStringLiteral("baseOperatingSystemName");
static const auto SoftwareVersion = QStringLiteral("softwareVersion");
static const auto SoftwareVersionId = QStringLiteral("softwareVersionId");
static const auto AdaptationVersion = QStringLiteral("adaptationVersion");
static const auto VendorName = QStringLiteral("vendorName");
static const auto VendorVersion = QStringLiteral("vendorVersion");

class AboutSettingsStub : public StubBase
{
public:
    virtual void AboutSettingsConstructor(QObject *parent);
    virtual void AboutSettingsDestructor();
    virtual QString operatingSystemName() const;
    virtual QString localizedSoftwareVersion() const;

    virtual QString wlanMacAddress() const;
    virtual QString serial() const;
    virtual QString localizedOperatingSystemName() const;
    virtual QString baseOperatingSystemName() const;
    virtual QString softwareVersion() const;
    virtual QString softwareVersionId() const;
    virtual QString adaptationVersion() const;

    virtual QString vendorName() const;
    virtual QString vendorVersion() const;
};


void AboutSettingsStub::AboutSettingsConstructor(QObject *parent)
{
    Q_UNUSED(parent)
}

void AboutSettingsStub::AboutSettingsDestructor()
{

}

QString AboutSettingsStub::operatingSystemName() const
{
    stubMethodEntered(OperatingSystemName);
    return stubReturnValue<QString>(OperatingSystemName);
}

QString AboutSettingsStub::localizedSoftwareVersion() const
{
    stubMethodEntered(LocalizedSoftwareVersion);
    return stubReturnValue<QString>(LocalizedSoftwareVersion);
}

QString AboutSettingsStub::wlanMacAddress() const
{
    stubMethodEntered(WlanMacAddress);
    return stubReturnValue<QString>(WlanMacAddress);
}

QString AboutSettingsStub::serial() const
{
    stubMethodEntered(Serial);
    return stubReturnValue<QString>(Serial);
}

QString AboutSettingsStub::localizedOperatingSystemName() const
{
    stubMethodEntered(LocalizedOperatingSystemName);
    return stubReturnValue<QString>(LocalizedOperatingSystemName);
}

QString AboutSettingsStub::baseOperatingSystemName() const
{
    stubMethodEntered(BaseOperatingSystemName);
    return stubReturnValue<QString>(BaseOperatingSystemName);
}

QString AboutSettingsStub::softwareVersion() const
{
    stubMethodEntered(SoftwareVersion);
    return stubReturnValue<QString>(SoftwareVersion);
}

QString AboutSettingsStub::softwareVersionId() const
{
    stubMethodEntered(SoftwareVersionId);
    return stubReturnValue<QString>(SoftwareVersionId);
}

QString AboutSettingsStub::adaptationVersion() const
{
    stubMethodEntered(AdaptationVersion);
    return stubReturnValue<QString>(AdaptationVersion);
}

QString AboutSettingsStub::vendorName() const
{
    stubMethodEntered(VendorName);
    return stubReturnValue<QString>(VendorName);
}

QString AboutSettingsStub::vendorVersion() const
{
    stubMethodEntered(VendorVersion);
    return stubReturnValue<QString>(VendorVersion);
}

AboutSettingsStub gDefaultAboutSettingsStub;
AboutSettingsStub *gAboutSettingsStub = &gDefaultAboutSettingsStub;

AboutSettings::AboutSettings(QObject *parent)
{
    gAboutSettingsStub->AboutSettingsConstructor(parent);
}

AboutSettings::~AboutSettings()
{
    gAboutSettingsStub->AboutSettingsDestructor();
}

QString AboutSettings::operatingSystemName() const
{
    return gAboutSettingsStub->operatingSystemName();
}

QString AboutSettings::localizedSoftwareVersion() const
{
    return gAboutSettingsStub->localizedSoftwareVersion();
}

QString AboutSettings::wlanMacAddress() const
{
    return gAboutSettingsStub->wlanMacAddress();
}

QString AboutSettings::serial() const
{
    return gAboutSettingsStub->serial();
}

QString AboutSettings::localizedOperatingSystemName() const
{
    return gAboutSettingsStub->localizedOperatingSystemName();
}

QString AboutSettings::baseOperatingSystemName() const
{
    return gAboutSettingsStub->baseOperatingSystemName();
}

QString AboutSettings::softwareVersion() const
{
    return gAboutSettingsStub->softwareVersion();
}

QString AboutSettings::softwareVersionId() const
{
    return gAboutSettingsStub->softwareVersionId();
}

QString AboutSettings::adaptationVersion() const
{
    return gAboutSettingsStub->adaptationVersion();
}

QString AboutSettings::vendorName() const
{
    return gAboutSettingsStub->vendorName();
}

QString AboutSettings::vendorVersion() const
{
    return gAboutSettingsStub->vendorVersion();
}

#endif
