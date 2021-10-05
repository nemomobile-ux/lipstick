/*
 * Copyright (C) 2021 Chupligin Sergey <neochapay@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QDebug>

#include "bluetoothagent.h"

#include <bluezqt/device.h>
#include <bluezqt/initmanagerjob.h>

BluetoothAgent::BluetoothAgent(QObject *parent)
    : BluezQt::Agent(parent)
    , m_device(nullptr)
    , m_manager(new BluezQt::Manager(this))
    , m_usableAdapter(nullptr)
    , m_connected(false)
    , m_available(false)
    , m_registerAgent(false)
{
    BluezQt::InitManagerJob *job = m_manager->init();
    job->start();

    connect(job, &BluezQt::InitManagerJob::result,
            this, &BluetoothAgent::initManagerJobResult);

    connect(m_manager,&BluezQt::Manager::usableAdapterChanged,
            this, &BluetoothAgent::usableAdapterChanged);

    connect(m_manager, &BluezQt::Manager::adapterAdded,
            this, &BluetoothAgent::calcAvailable);

    connect(m_manager, &BluezQt::Manager::adapterRemoved,
            this, &BluetoothAgent::calcAvailable);

    usableAdapterChanged(m_usableAdapter);
}

QDBusObjectPath BluetoothAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/org/nemomobile/lipstick/agent"));
}

BluezQt::Agent::Capability BluetoothAgent::capability() const
{
    return DisplayYesNo;
}

void BluetoothAgent::requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request)
{
    qDebug() << Q_FUNC_INFO;
    m_device = device;
    request.accept(QString());
}

void BluetoothAgent::displayPinCode(BluezQt::DevicePtr device, const QString &pinCode)
{
    qDebug() << Q_FUNC_INFO;
    m_device = device;
    emit showPinCode(pinCode);
}

void BluetoothAgent::requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &request)
{
    qDebug() << Q_FUNC_INFO;
    Q_UNUSED(request)
    m_device = device;
}

void BluetoothAgent::displayPasskey(BluezQt::DevicePtr device, const QString &passkey, const QString &entered)
{
    qDebug() << Q_FUNC_INFO;
    Q_UNUSED(passkey)
    Q_UNUSED(entered)
    m_device = device;
}

void BluetoothAgent::registerAgent()
{
    BluezQt::PendingCall *call = m_manager->registerAgent(this);

    qDebug() << "BT: bt agent registring";

    connect(call, &BluezQt::PendingCall::finished,
            this, &BluetoothAgent::registerAgentFinished);

}

void BluetoothAgent::pair(const QString &btMacAddress)
{
    m_device = m_manager->deviceForAddress(btMacAddress);
    if(!m_device) {
        qWarning() << "BT: Device not found";
        return;
    }

    BluezQt::PendingCall *pcall = m_device->pair();
    pcall->setUserData(btMacAddress);

    connect(pcall, &BluezQt::PendingCall::finished,
            this, &BluetoothAgent::pairFinished);
}

void BluetoothAgent::connectDevice(const QString &btMacAddress)
{
    m_device = m_manager->deviceForAddress(btMacAddress);
    if(!m_device) {
        qWarning() << "BT: Device not found";
        return;
    }

    m_device->connectToDevice();
}

void BluetoothAgent::pairFinished(BluezQt::PendingCall *call)
{
    QString btMacAddress = call->userData().toString();
    if(!call->error()) {
        m_device = m_manager->deviceForAddress(btMacAddress);
        if(m_device) {
            m_device->connectToDevice();
        }
    } else {
        emit error(call->errorText());
    }

}

void BluetoothAgent::unPair(const QString &btMacAddress)
{
    m_device = m_manager->deviceForAddress(btMacAddress);
    if(!m_device) {
        return;
    }

    m_usableAdapter->removeDevice(m_device);
}

void BluetoothAgent::usableAdapterChanged(BluezQt::AdapterPtr adapter)
{
    qDebug() << Q_FUNC_INFO;

    if(adapter && m_usableAdapter != adapter) {
        m_usableAdapter = adapter;

        connect(m_usableAdapter.data(), &BluezQt::Adapter::deviceChanged,
                this, &BluetoothAgent::updateConnectedStatus);
        updateConnectedStatus();
        emit adapterAdded(adapter);
        if(!m_registerAgent) {
            registerAgent();
        }
    }
}

void BluetoothAgent::requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request)
{
    Q_UNUSED(request);
    m_device = device;

    emit showRequiesDialog(m_device->address(),
                           m_device->name(),
                           passkey);

    connect(this, &BluetoothAgent::requestConfirmationAccept, this, [=] {
       request.accept();
    });

    connect(this, &BluetoothAgent::requestConfirmationReject, this, [=] {
        request.reject();
    });
}

void BluetoothAgent::requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request)
{
    qDebug() << Q_FUNC_INFO;
    m_device = device;
    request.accept();
}

void BluetoothAgent::authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request)
{
    qDebug() << Q_FUNC_INFO;
    Q_UNUSED(uuid);

    m_device = device;
    request.accept();
}

void BluetoothAgent::initManagerJobResult(BluezQt::InitManagerJob *job)
{
    if (job->error()) {
        qWarning() << "Error initializing Bluetooth manager:" << job->errorText();
    }
}

void BluetoothAgent::registerAgentFinished(BluezQt::PendingCall *call)
{
    if (call->error()) {
        qWarning() << "BT: registerAgent() call failed:" << call->errorText();
        return;
    }

    BluezQt::PendingCall *pcall = m_manager->requestDefaultAgent(this);
    connect(pcall, &BluezQt::PendingCall::finished,
            this, &BluetoothAgent::requestDefaultAgentFinished);

}

void BluetoothAgent::requestDefaultAgentFinished(BluezQt::PendingCall *call)
{
    if (call->error()) {
        qWarning() << "BT: requestDefaultAgent() call failed:" << call->errorText();
        emit error(call->errorText());
    }
    qDebug() << "BT: bt agent registring as system" << objectPath().path();
    m_registerAgent = true;
}

void BluetoothAgent::updateConnectedStatus()
{
    bool isSomebodyConnected = false;
    QList<QSharedPointer<BluezQt::Device>> devices = m_usableAdapter->devices();
    for (const QSharedPointer<BluezQt::Device> &device : qAsConst(devices)) {
        if (device->isConnected()) {
            isSomebodyConnected = true;
            break;
        }
    }

    if(m_connected != isSomebodyConnected) {
        m_connected = isSomebodyConnected;
        emit connectedChanged();
    }
}

void BluetoothAgent::calcAvailable(BluezQt::AdapterPtr adapter)
{
    Q_UNUSED(adapter)

    if(m_manager->adapters().count() > 0) {
        m_available = true;
    } else {
        m_available = false;
    }
}

bool BluetoothAgent::connected()
{
    return m_connected;
}

bool BluetoothAgent::available()
{
    return m_available;
}
