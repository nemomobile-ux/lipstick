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

#include "pulseaudiosinkinputmodel.h"
#include "pulseaudiocontrol.h"

#include <pulse/pulseaudio.h>
#include <QDebug>

PulseaudioSinkInputModel::PulseaudioSinkInputModel(QObject *parent) :
    QAbstractListModel(parent)
{
    m_pulseAudioControl = &PulseAudioControl::instance();

    hash.insert(Qt::UserRole ,QByteArray("sinkIndex"));
    hash.insert(Qt::UserRole+1 ,QByteArray("name"));
    hash.insert(Qt::UserRole+2 ,QByteArray("volume"));
    hash.insert(Qt::UserRole+3 ,QByteArray("mute"));
    hash.insert(Qt::UserRole+4 ,QByteArray("hasVolume"));
    hash.insert(Qt::UserRole+5 ,QByteArray("volumeWritable"));

    connect(m_pulseAudioControl, &PulseAudioControl::sinkInputAdded, this, &PulseaudioSinkInputModel::sinkAdded);
    connect(m_pulseAudioControl, &PulseAudioControl::sinkInputRemoved, this, &PulseaudioSinkInputModel::sinkRemoved);

    m_pulseAudioControl->update();
}

PulseaudioSinkInputModel::~PulseaudioSinkInputModel()
{

}

int PulseaudioSinkInputModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_sinkList.count();
}

QVariant PulseaudioSinkInputModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_sinkList.size()) {
        return QVariant();
    }

    PulseaudioSinkInput item = m_sinkList.at(index.row());
    if(role == Qt::UserRole)
    {
        return item.index;
    }
    else if(role == Qt::UserRole+1)
    {
        return item.name;
    }
    else if(role == Qt::UserRole+2)
    {
        return item.volume;
    }
    else if(role == Qt::UserRole+3)
    {
        return item.mute;
    }
    else if(role == Qt::UserRole+4)
    {
        return item.hasVolume;
    }
    else if(role == Qt::UserRole+5)
    {
        return item.volumeWritable;
    }

    return QVariant();
}

void PulseaudioSinkInputModel::sinkAdded(int index)
{
    pa_context_get_sink_input_info(m_pulseAudioControl->getContext(), index, sinkAddedCallBack, this);
}

void PulseaudioSinkInputModel::sinkRemoved(int i)
{
    int row = findBySinkInputID(i);
    if(row < 0) {
        return;
    }
    beginRemoveRows(QModelIndex(), row, row);
    m_sinkList.removeAt(row);
    endRemoveRows();
}

void PulseaudioSinkInputModel::sinkAddedCallBack(pa_context *, const pa_sink_input_info *i, int eol, void *userdata)
{
    PulseaudioSinkInputModel *pasim = static_cast<PulseaudioSinkInputModel*>(userdata);

    if(eol == 0) {
        int row = pasim->findBySinkInputID(i->index);
        if(row < 0) {
            pasim->addSink(*i);
        } else {
            pasim->updateSink(*i, row);
        }
    }
}

int PulseaudioSinkInputModel::findBySinkInputID(int sinkInputID)
{
    for(int i = 0; i < m_sinkList.count(); i++) {
        if(m_sinkList[i].index == sinkInputID) {
            return i;
        }
    }
    return -1;
}

void PulseaudioSinkInputModel::addSink(pa_sink_input_info i)
{
    beginInsertRows(QModelIndex(), m_sinkList.count(), m_sinkList.count());
    PulseaudioSinkInput input;
    input.index = i.index;
    input.name = i.name;
    input.volume = m_pulseAudioControl->paVolume2Percent(i.volume.values[0]);
    input.mute = i.mute;
    input.hasVolume = i.has_volume;
    input.volumeWritable = i.volume_writable;

    m_sinkList.append(input);
    endInsertRows();
}

void PulseaudioSinkInputModel::updateSink(pa_sink_input_info i, int row)
{
    PulseaudioSinkInput input = m_sinkList.at(row);
    input.index = i.index;
    input.name = i.name;
    input.volume = m_pulseAudioControl->paVolume2Percent(i.volume.values[0]);
    input.mute = i.mute;
    input.hasVolume = i.has_volume;
    input.volumeWritable = i.volume_writable;
    m_sinkList.replace(row, input);
    emit dataChanged(index(row), index(row));
}
