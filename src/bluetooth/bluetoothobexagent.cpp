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

#include "bluetoothobexagent.h"

#include <QDir>
#include <QDBusObjectPath>
#include <QStandardPaths>

#include <BluezQt/ObexManager>
#include <BluezQt/ObexSession>

BluetoothObexAgent::BluetoothObexAgent(QObject *parent)
    : BluezQt::ObexAgent(parent)
{
    BluezQt::PendingCall* startCall = BluezQt::ObexManager::startService();

    connect(startCall, &BluezQt::PendingCall::finished, this, &BluetoothObexAgent::startServiceFinished);
}

BluetoothObexAgent::~BluetoothObexAgent()
{

}

QDBusObjectPath BluetoothObexAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/org/nemomobile/lipstick/obexagent"));
}

void BluetoothObexAgent::authorizePush(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request)
{
    emit showRequiesDialog(session->destination(), transfer->name());
    connect(this, &BluetoothObexAgent::requestConfirmationReject, this, [=]() {
        request.reject();
    });

    connect(this, &BluetoothObexAgent::requestConfirmationAccept, this, [=]() {
        QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/obexd";
        QString transferName = transfer->name();

        if(!QDir(cachePath).exists()) {
            QDir(cachePath).mkpath(cachePath);
        }

        QString temporaryPath = cachePath + transferName;
        int number = 0;

        while (QFile::exists(temporaryPath)) {
            temporaryPath = cachePath + transferName + "_" + QString::number(number);
            number++;
        }
        m_temporaryPath = temporaryPath;
        m_transferName = transferName;

        connect(transfer.data(), &BluezQt::ObexTransfer::statusChanged, this, &BluetoothObexAgent::obexDataTransferFinished);
    });
}

void BluetoothObexAgent::startServiceFinished(BluezQt::PendingCall *call)
{
    if(call->error()) {
        qWarning() << Q_FUNC_INFO << call->errorText();
        return;
    }

    m_obexManager = new BluezQt::ObexManager();
    BluezQt::InitObexManagerJob* initObexManagerJob = m_obexManager->init();

    connect(initObexManagerJob, &BluezQt::InitObexManagerJob::result, this, &BluetoothObexAgent::obexManagerStartResult);
    initObexManagerJob->start();
}

void BluetoothObexAgent::obexManagerStartResult(BluezQt::InitObexManagerJob *job)
{
    if(job->error()) {
        qWarning() << Q_FUNC_INFO << job->errorText();
        return;
    }
    m_obexManager->registerAgent(this);
}

void BluetoothObexAgent::obexDataTransferFinished(BluezQt::ObexTransfer::Status status)
{
    if(status == BluezQt::ObexTransfer::Complete) {
        QString downloadsDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/bluethooth";
        if(!QDir(downloadsDir).exists()) {
            QDir::root().mkpath(downloadsDir);
        }

        QString resultPath = QDir(downloadsDir).absoluteFilePath(m_transferName);
        int number = 0;
        while (QFile::exists(resultPath)) {
            resultPath = QDir(downloadsDir).absoluteFilePath(QStringLiteral("%1 (%2)").arg(m_transferName, number));
            number++;
        }

        QFile::rename(m_transferName, resultPath);
        emit transferFinished(resultPath);
    }else if(status == BluezQt::ObexTransfer::Error) {
        emit transferError();
    }
}
