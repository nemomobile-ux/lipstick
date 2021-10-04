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

#ifndef BLUETOOTHAGENT_H
#define BLUETOOTHAGENT_H

#include <bluezqt/agent.h>
#include <bluezqt/adapter.h>
#include <bluezqt/request.h>
#include <bluezqt/manager.h>
#include <bluezqt/pendingcall.h>

#include "lipstickglobal.h"

class LIPSTICK_EXPORT BluetoothAgent : public BluezQt::Agent
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)

public:
    BluetoothAgent(QObject *parent = Q_NULLPTR);
    QDBusObjectPath objectPath() const;
    Capability capability() const;

    void requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request);

    Q_INVOKABLE void pair(const QString &btMacAddress);
    Q_INVOKABLE void unPair(const QString &btMacAddress);
    Q_INVOKABLE void connectDevice(const QString &btMacAddress);

    bool connected();
    bool available();

public slots:
    void registerAgent();

signals:
    void adapterAdded(const BluezQt::AdapterPtr adapter);
    void showRequiesDialog(const QString btMacAddres, const QString name, const QString code);
    void connectedChanged();
    void availableChanged();

private:
    void initManagerJobResult(BluezQt::InitManagerJob *job);
    void registerAgentFinished(BluezQt::PendingCall *call);
    void requestDefaultAgentFinished(BluezQt::PendingCall *call);

    void usableAdapterChanged(BluezQt::AdapterPtr adapter);
    void connectToDevice(BluezQt::PendingCall *call);
    void updateConnectedStatus();

    void calcAvailable(BluezQt::AdapterPtr adapter);

    BluezQt::Manager *m_manager;
    BluezQt::AdapterPtr m_usableAdapter;
    bool m_connected;
    bool m_available;
    bool m_registerAgent;
};

#endif // BLUETOOTHAGENT_H
