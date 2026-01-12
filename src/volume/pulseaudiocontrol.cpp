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

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusArgument>
#include <QDBusServiceWatcher>
#include <QTimer>


PulseAudioControl::PulseAudioControl(QObject *parent) :
    QObject(parent)
    , m_loop(new PulseAudioPrivateLoop(this))
    , m_buttonHandler(new MceButtonHandler(this))
{
    connect(m_loop, &PulseAudioPrivateLoop::volumeChanged,
            this, &PulseAudioControl::volumeChanged,
            Qt::QueuedConnection);


    int step = 5; // TODO

    connect(m_buttonHandler, &MceButtonHandler::volumeUp, this, [=]() {
        int newVol = qMin(m_loop->currentVolume() + step, 100);
        m_loop->setVolume(newVol);
    });

    connect(m_buttonHandler, &MceButtonHandler::volumeDown, this, [=]() {
        int newVol = qMax(m_loop->currentVolume() - step, 0);
        m_loop->setVolume(newVol);
    });

    connect(m_buttonHandler, &MceButtonHandler::volumeMute, this, [=]() {
        m_loop->setMute(!m_loop->isMuted());
    });


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
