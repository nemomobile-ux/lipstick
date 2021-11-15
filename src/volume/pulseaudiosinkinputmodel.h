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

#ifndef PULSEAUDIOSINKINPUTMODEL_H
#define PULSEAUDIOSINKINPUTMODEL_H

#include <QObject>
#include <QAbstractListModel>

#include "pulseaudiocontrol.h"
#include "lipstickglobal.h"

class LIPSTICK_EXPORT PulseaudioSinkInputModel:public QAbstractListModel
{
    Q_OBJECT
/*
 * Reworked pa_sink_input_info from here
 * https://gitlab.freedesktop.org/pulseaudio/pulseaudio/-/blob/master/src/pulse/introspect.h#L646
*/
    struct PulseaudioSinkInput{
        int index;
        QString name;
        int volume;
        bool mute;
        bool hasVolume;
        bool volumeWritable;
    };

public:
    explicit PulseaudioSinkInputModel(QObject *parent = 0);
    virtual ~PulseaudioSinkInputModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const {return hash;}

protected:
    enum InitializationMode {
        DeferInitialization
    };

private slots:
    void sinkAdded(int index);
    void sinkRemoved(int index);

private:
    QHash<int,QByteArray> hash;
    QList<PulseaudioSinkInput> m_sinkList;

    static void sinkAddedCallBack(pa_context *, const pa_sink_input_info *i, int eol, void *userdata);
    int findBySinkInputID(int sinkInputID);

    void addSink(pa_sink_input_info i);
    void updateSink(pa_sink_input_info i, int row);

    PulseAudioControl *m_pulseAudioControl;
};

#endif // PULSEAUDIOSINKINPUTMODEL_H
