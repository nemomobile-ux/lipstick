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

#include "pulseaudiocontrol.h"
#include "dbus-gmain/dbus-gmain.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusArgument>
#include <QDBusServiceWatcher>
#include <QTimer>


#define DBUS_ERR_CHECK(err) \
    if (dbus_error_is_set(&err)) \
    { \
        qWarning() << err.message; \
        dbus_error_free(&err); \
    }

static const char *VOLUME_SERVICE = "com.Meego.MainVolume2";
static const char *VOLUME_PATH = "/com/meego/mainvolume2";
static const char *VOLUME_INTERFACE = "com.Meego.MainVolume2";

#define PA_RECONNECT_TIMEOUT_MS (2000)

PulseAudioControl::PulseAudioControl(QObject *parent) :
    QObject(parent)
{
    m_loop = new PulseAudioPrivateLoop(this);
    connect(m_loop, &PulseAudioPrivateLoop::volumeChanged,
            this, &PulseAudioControl::volumeChanged,
            Qt::QueuedConnection);
    m_loop->start();
}

PulseAudioControl::~PulseAudioControl()
{
}

void PulseAudioControl::update()
{
    qDebug() << Q_FUNC_INFO;
}

void PulseAudioControl::setSteps(quint32 currentStep, quint32 stepCount)
{
    // The pulseaudio API reports the step count (starting from 0), so the maximum volume is stepCount - 1
    emit volumeChanged(currentStep, stepCount - 1);
}

void PulseAudioControl::setVolume(int volume)
{
    m_loop->setVolume(volume);
}
