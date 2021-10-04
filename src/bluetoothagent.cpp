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

# include <bluezqt/device.h>
# include <bluezqt/initmanagerjob.h>

BluetoothAgent::BluetoothAgent(QObject *parent)
    : BluezQt::Agent(parent)
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

void BluetoothAgent::registerAgent()
{
    BluezQt::PendingCall *call = m_manager->registerAgent(this);

    qDebug() << "BT: bt agent registring";

    connect(call, &BluezQt::PendingCall::finished,
            this, &BluetoothAgent::registerAgentFinished);

}

void BluetoothAgent::pair(const QString &btMacAddress)
{
    BluezQt::DevicePtr device = m_manager->deviceForAddress(btMacAddress);
    if(!device)
    {
        qWarning() << "BT: Device not found";
        return;
    }

    BluezQt::PendingCall *pcall = device->pair();
    pcall->setUserData(btMacAddress);

    connect(pcall, &BluezQt::PendingCall::finished,
            this, &BluetoothAgent::connectToDevice);
}

void BluetoothAgent::connectDevice(const QString &btMacAddress)
{
    BluezQt::DevicePtr device = m_manager->deviceForAddress(btMacAddress);
    if(!device)
    {
        qWarning() << "BT: Device not found";
        return;
    }

    device->connectToDevice();
}

void BluetoothAgent::connectToDevice(BluezQt::PendingCall *call)
{
    QString btMacAddress = call->userData().toString();
    if(!call->error()) {
        BluezQt::DevicePtr device = m_manager->deviceForAddress(btMacAddress);
        if(device) {
            device->connectToDevice();
        }
    }
}

void BluetoothAgent::unPair(const QString &btMacAddress)
{
    BluezQt::DevicePtr device = m_manager->deviceForAddress(btMacAddress);
    if(!device)
    {
        return;
    }

    m_usableAdapter->removeDevice(device);
}

void BluetoothAgent::usableAdapterChanged(BluezQt::AdapterPtr adapter)
{
    if(adapter && m_usableAdapter != adapter)
    {
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

    emit showRequiesDialog(device->address() ,
                           device->name() ,
                           passkey);
}

void BluetoothAgent::initManagerJobResult(BluezQt::InitManagerJob *job)
{
    if (job->error())
    {
        qWarning() << "Error initializing Bluetooth manager:" << job->errorText();
    }
}

void BluetoothAgent::registerAgentFinished(BluezQt::PendingCall *call)
{
    if (call->error())
    {
        qWarning() << "BT: registerAgent() call failed:" << call->errorText();
        return;
    }

    BluezQt::PendingCall *pcall = m_manager->requestDefaultAgent(this);
    connect(pcall, &BluezQt::PendingCall::finished,
            this, &BluetoothAgent::requestDefaultAgentFinished);

}

void BluetoothAgent::requestDefaultAgentFinished(BluezQt::PendingCall *call)
{
    if (call->error())
    {
        qWarning() << "BT: requestDefaultAgent() call failed:" << call->errorText();
    }
    qDebug() << "BT: bt agent registring as system" << objectPath().path();
    m_registerAgent = true;
}

void BluetoothAgent::updateConnectedStatus()
{
    bool isSomebodyConnected = false;
    for (const auto &device : m_usableAdapter->devices()) {
        if (device->isConnected()) {
            isSomebodyConnected = true;
            break;
        }
    }

    if(m_connected != isSomebodyConnected)
    {
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
