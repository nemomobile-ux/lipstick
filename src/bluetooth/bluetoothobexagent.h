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


#ifndef BLUETOOTHOBEXAGENT_H
#define BLUETOOTHOBEXAGENT_H

#include <BluezQt/ObexAgent>
#include <BluezQt/ObexTransfer>
#include <BluezQt/PendingCall>
#include <BluezQt/InitObexManagerJob>

#include "lipstickglobal.h"

class LIPSTICK_EXPORT BluetoothObexAgent : public BluezQt::ObexAgent {
    Q_OBJECT

public:
    explicit BluetoothObexAgent(QObject* parent = nullptr);
    ~BluetoothObexAgent();

    QDBusObjectPath objectPath() const override;
    void authorizePush(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString>& request) override;

signals:
    void showRequiesDialog(const QString deviceName, const QString fileName);
    void requestConfirmationAccept();
    void requestConfirmationReject();
    void transferError();
    void transferFinished(QString resultPath);

private slots:
    void startServiceFinished(BluezQt::PendingCall *call);
    void obexManagerStartResult(BluezQt::InitObexManagerJob *job);
    void obexDataTransferFinished(BluezQt::ObexTransfer::Status status);

private:
    BluezQt::ObexManager *m_obexManager;
    QString m_temporaryPath;
    QString m_transferName;

};

#endif // BLUETOOTHOBEXAGENT_H
