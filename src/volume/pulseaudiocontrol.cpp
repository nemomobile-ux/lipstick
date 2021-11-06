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

#include <pulse/glib-mainloop.h>
#include <pulse/operation.h>

#include "pulseaudiocontrol.h"
#include <QDebug>

PulseAudioControl::PulseAudioControl(QObject *parent) :
    QObject(parent),
    m_paContext(nullptr),
    m_paAPI(nullptr)
{
    qDebug() << Q_FUNC_INFO;

    pa_glib_mainloop *m = pa_glib_mainloop_new(g_main_context_default());
    g_assert(m);
    m_paAPI = pa_glib_mainloop_get_api(m);
}

PulseAudioControl::~PulseAudioControl()
{
}

void PulseAudioControl::openConnection()
{
    qDebug() << Q_FUNC_INFO;
    pa_proplist *proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "lipstick");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "org.PulseAudio.lipstick");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "audio-card");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, "0.1");

    m_paContext = pa_context_new_with_proplist(m_paAPI, nullptr, proplist);
    g_assert(m_paContext);

    pa_proplist_free(proplist);
    pa_context_set_state_callback(m_paContext, stateCallBack, this);

    if (pa_context_connect(m_paContext, nullptr, PA_CONTEXT_NOFAIL, nullptr) < 0) {
        if (pa_context_errno(m_paContext) == PA_ERR_INVALID) {
            emit pulseConnectFailed();
        }
    } else {
        qDebug() << "Connection to PulseAudio success";
    }
}

void PulseAudioControl::update()
{
    openConnection();
}

int PulseAudioControl::paVolume2Percent(pa_volume_t vol)
{
    if(vol > PA_VOLUME_UI_MAX) {
        vol = PA_VOLUME_UI_MAX;
    }
    return qRound(static_cast<double>(vol - PA_VOLUME_MUTED) / PA_VOLUME_NORM * 100);
}

pa_volume_t PulseAudioControl::percent2PaVolume(int percent)
{
    return PA_VOLUME_MUTED + qRound(static_cast<double>(percent) / 100 * PA_VOLUME_NORM);
}

void PulseAudioControl::stateCallBack(pa_context *context, void *userdata)
{
    g_assert(context);
    PulseAudioControl *pac = static_cast<PulseAudioControl*>(userdata);

    switch (pa_context_get_state(context)) {
    case PA_CONTEXT_UNCONNECTED:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        break;

    case PA_CONTEXT_READY:
        qDebug() << "PA_CONTEXT_READY";
        pa_operation *o;
        pa_context_set_subscribe_callback(context, pac->subscribeCallBack, pac);

        if(!(o = pa_context_subscribe(context, (pa_subscription_mask_t)
                                      (PA_SUBSCRIPTION_MASK_SINK|
                                       PA_SUBSCRIPTION_MASK_SOURCE|
                                       PA_SUBSCRIPTION_MASK_SINK_INPUT|
                                       PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT|
                                       PA_SUBSCRIPTION_MASK_CLIENT|
                                       PA_SUBSCRIPTION_MASK_SERVER|
                                       PA_SUBSCRIPTION_MASK_CARD), nullptr, nullptr))) {
            qWarning("PULSEAUDIOCONTROL: pa_context_subscribe() failed");
            return;
        }
        pa_operation_unref(o);
        pa_context_get_server_info(context, pac->serverInfoCallback, pac);
        pa_context_get_client_info_list(context, pac->clientCallback, pac);
        pa_context_get_card_info_list(context, pac->cardCallBack, pac);
        pa_context_get_sink_info_list(context, pac->sinkCallBack, pac);
        pa_context_get_source_info_list(context, pac->sourceCallBack, pac);
        pa_context_get_sink_input_info_list(context, pac->sinkInputCallBack, pac);
        pa_context_get_source_output_info_list(context, pac->sourceOutputCallBack, pac);

        break;
    case PA_CONTEXT_FAILED:
        pa_context_unref(context);
        pac->m_paContext = nullptr;
        break;
    case PA_CONTEXT_TERMINATED:
    default:
        qWarning() << "Something wrong!!!";
        break;
    }
}

void PulseAudioControl::subscribeCallBack(pa_context *context, pa_subscription_event_type_t t, uint32_t index, void *userdata)
{
    PulseAudioControl *pac = static_cast<PulseAudioControl*>(userdata);
    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
    case PA_SUBSCRIPTION_EVENT_SINK:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            qDebug() << "Remove sink" << index;
            pac->m_sinksOutput.removeAt(index);
        } else {
            pa_operation *o;
            if (!(o = pa_context_get_sink_info_by_index(context, index, pac->sinkCallBack, pac))) {
                qWarning("pa_context_get_sink_info_by_index() failed");
                return;
            }
            pa_operation_unref(o);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SOURCE:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            qDebug() << "Remove source" << index;
        } else {
            pa_operation *o;
            if (!(o = pa_context_get_source_info_by_index(context, index, pac->sourceCallBack, pac))) {
                qWarning("pa_context_get_source_info_by_index() failed");
                return;
            }
            pa_operation_unref(o);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            qDebug() << "Remove input" << index;
            pac->m_sinksInput.removeAt(index);
        } else {
            pa_operation *o;
            if (!(o = pa_context_get_sink_input_info(context, index, pac->sinkInputCallBack, pac))) {
                qWarning("pa_context_get_sink_input_info() failed");
                return;
            }
            pa_operation_unref(o);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            pa_operation *o;
            if (!(o = pa_context_get_source_output_info(context, index, pac->sourceOutputCallBack, pac))) {
                qDebug("pa_context_get_sink_input_info() failed");
                return;
            }
            pa_operation_unref(o);
        } else {
            qDebug() << "Added output" << index;
        }
        break;
    case PA_SUBSCRIPTION_EVENT_CLIENT:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            qDebug() << "Remove client" << index;
        } else {
            pa_operation *o;
            if(!(o = pa_context_get_client_info(context, index, pac->clientCallback, pac))) {
                qWarning("pa_context_get_client_info() failed");
                return;
            }
            pa_operation_unref(o);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_CARD:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            qDebug() << "Remove card" << index;
        } else {
            pa_operation *o;
            if (!(o = pa_context_get_card_info_by_index(context, index, pac->cardCallBack, pac))) {
                qWarning("pa_context_get_card_info_by_index() failed");
                return;
            }
            pa_operation_unref(o);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SERVER:
        pa_operation *o;
        if (!(o = pa_context_get_server_info(context, pac->serverInfoCallback, pac))) {
            qDebug("pa_context_get_server_info() failed");
            return;
        }
        pa_operation_unref(o);
        break;
    }
}

void PulseAudioControl::clientCallback(pa_context *, const pa_client_info *i, int eol, void *userdata)
{
    PulseAudioControl *pac = static_cast<PulseAudioControl*>(userdata);
    if(eol < 0) {
        if (pa_context_errno(pac->m_paContext) == PA_ERR_NOENTITY) {
            return;
        }
        qWarning("Client callback failure");
    }

    if(eol == 0) {
        qDebug() << "=========== Added client ==================";
        qDebug() << "Name:      " << i->name;
        qDebug() << "Driver:    " << i->driver;
    }
}

void PulseAudioControl::sinkCallBack(pa_context *, const pa_sink_info *i, int eol, void *userdata)
{
    PulseAudioControl *pac = static_cast<PulseAudioControl*>(userdata);
    if(eol < 0) {
        if (pa_context_errno(pac->m_paContext) == PA_ERR_NOENTITY) {
            return;
        }
        qWarning("Sink callback failure");
    }

    if(eol == 0) {
        qDebug() << "=========== Added sink output ============";
        qDebug() << "Name:          " << i->name;
        qDebug() << "Driver:        " << i->driver;
        qDebug() << "Description:   " << i->description;
        qDebug() << "Muted:         " << i->mute;

        if(i->name == pac->m_defaultSinkName) {
            pac->m_defaultSink = *i;
            /*
             * if default channel mutted - unmute it
            */
            pa_context_set_sink_mute_by_index(pac->m_paContext, i->index, 0, nullptr, nullptr);
            emit pac->volumeChanged(pac->paVolume2Percent(i->volume.values[0]), pac->paVolume2Percent(PA_VOLUME_MAX));
        }

        pac->m_sinksOutput.insert(i->index, *i);
    }
}

void PulseAudioControl::sourceCallBack(pa_context *, const pa_source_info *i, int eol, void *userdata)
{
    PulseAudioControl *pac = static_cast<PulseAudioControl*>(userdata);
    if(eol < 0) {
        if (pa_context_errno(pac->m_paContext) == PA_ERR_NOENTITY) {
            return;
        }
        qWarning("Source callback failure");
    }

    if(eol == 0) {
        qDebug() << "=========== Added source ==================";
        qDebug() << "Name:          " << i->name;
        qDebug() << "Driver:        " << i->driver;
        qDebug() << "Description:   " << i->description;
        qDebug() << "Muted:         " << i->mute;
    }
}

void PulseAudioControl::sinkInputCallBack(pa_context *, const pa_sink_input_info *i, int eol, void *userdata)
{
    PulseAudioControl *pac = static_cast<PulseAudioControl*>(userdata);
    if(eol < 0) {
        if (pa_context_errno(pac->m_paContext) == PA_ERR_NOENTITY) {
            return;
        }
        qWarning("Sink input callback failure");
    }

    if(eol == 0) {
        qDebug() << "=========== Added sink input ==============";
        qDebug() << "ID:            " << i->index;
        qDebug() << "Name:          " << i->name;
        qDebug() << "Driver:        " << i->driver;

        pac->m_sinksInput.insert(i->index, *i);
    }
}

void PulseAudioControl::sourceOutputCallBack(pa_context *, const pa_source_output_info *i, int eol, void *userdata)
{
    PulseAudioControl *pac = static_cast<PulseAudioControl*>(userdata);
    if(eol < 0) {
        if (pa_context_errno(pac->m_paContext) == PA_ERR_NOENTITY) {
            return;
        }
        qWarning("Source output callback failure");
    }

    if(eol == 0) {
        qDebug() << "==========> Added source output <=============";
        qDebug() << "Name:          " << i->name;
        qDebug() << "Driver:        " << i->driver;
    }
}

void PulseAudioControl::serverInfoCallback(pa_context *, const pa_server_info *i, void *userdata)
{
    PulseAudioControl *pac = static_cast<PulseAudioControl*>(userdata);
    Q_UNUSED(pac);

    if(!i) {
        qWarning("Server info callback failure");
        return;
    }

    qDebug() << "=========== Servier info  ==============";
    qDebug() << "default_sink_name:     " << i->default_sink_name;
    qDebug() << "default_source_name:   " << i->default_source_name;

    pac->m_defaultSinkName = i->default_sink_name;
    pac->m_defaultSourceName = i->default_source_name;
}

void PulseAudioControl::cardCallBack(pa_context *, const pa_card_info *i, int eol, void *userdata)
{
    PulseAudioControl *pac = static_cast<PulseAudioControl*>(userdata);
    if(eol < 0) {
        if (pa_context_errno(pac->m_paContext) == PA_ERR_NOENTITY) {
            return;
        }
        qWarning("Source output callback failure");
    }

    if(eol == 0) {
        qDebug() << "=========== Added card ==============";
        qDebug() << "Name:          " << i->name;
        qDebug() << "Driver:        " << i->driver;
    }
}

void PulseAudioControl::setVolumeCallBack(pa_context *, int success, void *)
{
    if(success < 0) {
        qWarning() << "Set volume falled";
    }
}

void PulseAudioControl::setVolume(int volume)
{
    pa_operation* o;

    pa_cvolume cvol;
    cvol.channels = m_defaultSink.volume.channels;

    if(percent2PaVolume(volume) > percent2PaVolume(PA_VOLUME_NORM)) {
        emit highVolume(paVolume2Percent(PA_VOLUME_NORM));
    }

    for(int i=0; i < cvol.channels; i++) {
        cvol.values[i] = percent2PaVolume(volume);
    }

    if (!(o = pa_context_set_sink_volume_by_name(m_paContext, m_defaultSinkName.toUtf8(), &cvol, setVolumeCallBack, nullptr))) {
            qWarning("pa_context_set_source_volume_by_name FAILED!");
    }
    pa_operation_unref(o);
}
