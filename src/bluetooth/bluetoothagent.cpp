/*
 * Copyright (C) 2021-2024 Chupligin Sergey <neochapay@gmail.com>
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

#include "bluetoothagent.h"
#include "logging.h"
#include "homewindow.h"
#include "bluetoothagent.h"
#include "lipstickqmlpath.h"
#include "utilities/closeeventeater.h"


#include <bluezqt/device.h>
#include <bluezqt/initmanagerjob.h>

#include <QScreen>
#include <QGuiApplication>
#include <QPoint>
#include <QTimer>

BluetoothAgent::BluetoothAgent(QObject *parent)
    : BluezQt::Agent(parent)
    , m_device(nullptr)
    , m_manager(new BluezQt::Manager(this))
    , m_usableAdapter(nullptr)
    , m_connected(false)
    , m_available(false)
    , m_registerAgent(false)
    , m_requestDialogWindow(nullptr)
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
    QTimer::singleShot(0, this, SLOT(createWindow()));
}

BluetoothAgent::~BluetoothAgent()
{
    delete m_requestDialogWindow;
}

QDBusObjectPath BluetoothAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/org/nemomobile/lipstick/agent"));
}

BluezQt::Agent::Capability BluetoothAgent::capability() const
{
    return DisplayYesNo;
}

void BluetoothAgent::requestPinCode(BluezQt::DevicePtr device,
                                    const BluezQt::Request<QString> &request)
{
    m_device = device;
    request.accept(QString());
}

void BluetoothAgent::displayPinCode(BluezQt::DevicePtr device,
                                    const QString &pinCode)
{
    m_device = device;
    emit showPinCode(pinCode);
}

void BluetoothAgent::requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &request)
{
    Q_UNUSED(request)
    m_device = device;
}

void BluetoothAgent::displayPasskey(BluezQt::DevicePtr device, const QString &passkey, const QString &entered)
{
    Q_UNUSED(passkey)
    Q_UNUSED(entered)
    m_device = device;
}

void BluetoothAgent::registerAgent()
{
    BluezQt::PendingCall *call = m_manager->registerAgent(this);

    qCDebug(lcLipstickBtAgentLog) << "BT: bt agent registring";

    connect(call, &BluezQt::PendingCall::finished,
            this, &BluetoothAgent::registerAgentFinished);

}

void BluetoothAgent::createWindow()
{
    m_requestDialogWindow = new HomeWindow();
    m_requestDialogWindow->setGeometry(QRect(QPoint(), QGuiApplication::primaryScreen()->size()));
    m_requestDialogWindow->setCategory(QLatin1String("notification"));
    m_requestDialogWindow->setWindowTitle("Confirmation dialog");
    m_requestDialogWindow->setContextProperty("initialSize", QGuiApplication::primaryScreen()->size());
    m_requestDialogWindow->setSource(QmlPath::to("bluetooth/RequestConfirmationDialog.qml"));
    m_requestDialogWindow->installEventFilter(new CloseEventEater(this));
}

void BluetoothAgent::pair(const QString &btMacAddress)
{
    m_device = m_manager->deviceForAddress(btMacAddress);
    if(!m_device) {
        qCWarning(lcLipstickBtAgentLog) << "BT: Device not found";
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
        qCWarning(lcLipstickBtAgentLog) << "BT: Device not found";
        return;
    }

    m_device->connectToDevice();
}

void BluetoothAgent::pairFinished(BluezQt::PendingCall *call)
{
    if(call->error()) {
        qCWarning(lcLipstickBtAgentLog) << "BT: pairFinished error" << call->errorText();
        emit error(call->errorText());
    } else {
        QString btMacAddress = call->userData().toString();
        m_device = m_manager->deviceForAddress(btMacAddress);
        if(m_device) {
            m_device->connectToDevice();
        }
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

void BluetoothAgent::requestConfirmation(BluezQt::DevicePtr device,
                                         const QString &passkey,
                                         const BluezQt::Request<> &request)
{
    m_device = device;
    m_deviceAddress = m_device->address();
    m_deviceName = m_device->name();
    m_devicePassKey = passkey;

    emit showRequiesDialog();

    connect(this, &BluetoothAgent::requestConfirmationAccept, this, [=] {
        request.accept();
    });

    connect(this, &BluetoothAgent::requestConfirmationReject, this, [=] {
        request.reject();
    });
}

void BluetoothAgent::requestAuthorization(BluezQt::DevicePtr device,
                                          const BluezQt::Request<> &request)
{
    m_device = device;
    request.accept();
}

void BluetoothAgent::authorizeService(BluezQt::DevicePtr device,
                                      const QString &uuid,
                                      const BluezQt::Request<> &request)
{
    Q_UNUSED(uuid);

    m_device = device;
    request.accept();
}

void BluetoothAgent::initManagerJobResult(BluezQt::InitManagerJob *job)
{
    if (job->error()) {
        qCWarning(lcLipstickBtAgentLog) << "Error initializing Bluetooth manager:" << job->errorText();
    }
}

void BluetoothAgent::registerAgentFinished(BluezQt::PendingCall *call)
{
    if (call->error()) {
        qCWarning(lcLipstickBtAgentLog) << "BT: registerAgent() call failed:" << call->errorText();
        return;
    }

    BluezQt::PendingCall *pcall = m_manager->requestDefaultAgent(this);
    connect(pcall, &BluezQt::PendingCall::finished,
            this, &BluetoothAgent::requestDefaultAgentFinished);

}

void BluetoothAgent::requestDefaultAgentFinished(BluezQt::PendingCall *call)
{
    if (call->error()) {
        qCWarning(lcLipstickBtAgentLog) << "BT: requestDefaultAgent() call failed:" << call->errorText();
        emit error(call->errorText());
    }
    qCDebug(lcLipstickBtAgentLog) << "BT: bt agent registring as system" << objectPath().path();
    m_registerAgent = true;
}

void BluetoothAgent::updateConnectedStatus()
{
    bool isSomebodyConnected = false;
    QList<QSharedPointer<BluezQt::Device>> devices = m_usableAdapter->devices();
    for (const QSharedPointer<BluezQt::Device> &device : std::as_const(devices)) {
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

bool BluetoothAgent::windowVisible() const
{
    return m_requestDialogWindow != 0 && m_requestDialogWindow->isVisible();
}

void BluetoothAgent::setWindowVisible(bool visible)
{
    if(visible) {
        if(m_requestDialogWindow && !m_requestDialogWindow->isVisible()) {
            m_requestDialogWindow->show();
            emit windowVisibleChanged();
        }
    } else if(m_requestDialogWindow != 0 && m_requestDialogWindow->isVisible()) {
        m_requestDialogWindow->hide();
        emit windowVisibleChanged();
    }
}

QString BluetoothAgent::deviceAddress() const
{
    return m_deviceAddress;
}

void BluetoothAgent::setDeviceAddress(const QString &newDeviceAddress)
{
    if (m_deviceAddress == newDeviceAddress)
        return;
    m_deviceAddress = newDeviceAddress;
    emit deviceAddressChanged();
}

QString BluetoothAgent::devicePassKey() const
{
    return m_devicePassKey;
}

void BluetoothAgent::setDevicePassKey(const QString &newDevicePassKey)
{
    if (m_devicePassKey == newDevicePassKey)
        return;
    m_devicePassKey = newDevicePassKey;
    emit devicePassKeyChanged();
}

QString BluetoothAgent::deviceName() const
{
    return m_deviceName;
}

void BluetoothAgent::setDeviceName(const QString &newDeviceName)
{
    if (m_deviceName == newDeviceName)
        return;
    m_deviceName = newDeviceName;
    emit deviceNameChanged();
}
