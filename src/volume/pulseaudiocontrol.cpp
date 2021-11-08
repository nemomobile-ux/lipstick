/***************************************************************************
**
** Copyright (c) 2012 Jolla Ltd.
** Copyright (c) 2021 Chupligin Sregey (NeoChapay) <neochapay@gmail.com>
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

#include <pulse/operation.h>

#include "pulseaudiocontrol.h"
#include <QDebug>

static PulseAudioControl *pulseAudioControlInstance = 0;

PulseAudioControl::PulseAudioControl(QObject *parent) :
    QObject(parent)
  , m_paContext(nullptr)
  , m_paAPI(nullptr)
  , m_defaultSinkName("")
  , m_defaultSourceName("")
  , defaultSinkChannels(-1)
{
    QMutexLocker locker(&lock);

    m = pa_glib_mainloop_new(g_main_context_default());
    g_assert(m);
    m_paAPI = pa_glib_mainloop_get_api(m);

    connect(this, &PulseAudioControl::defaultSinkNameChanged, this, &PulseAudioControl::setupDefaultSink);
}

PulseAudioControl::~PulseAudioControl()
{
    pa_context_unref(m_paContext);
    pa_glib_mainloop_free(m);
}

PulseAudioControl &PulseAudioControl::instance()
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    if(!pulseAudioControlInstance) {
        pulseAudioControlInstance = new PulseAudioControl;
    }
    return *pulseAudioControlInstance;
}

void PulseAudioControl::openConnection()
{
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
    if(m_paContext == nullptr) {
        openConnection();
    }
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
            pac->m_sinks.removeAt(index);
            emit pac->sinkRemoved(index);
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
            pac->m_sources.removeAt(index);
            emit pac->sourceRemoved(index);
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
            pac->m_sinksInput.removeAt(index);
            emit pac->sinkInputRemoved(index);
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
            pac->m_sourceOutputs.removeAt(index);
            emit pac->sourceOutputRemoved(index);
        } else {
            pa_operation *o;
            if (!(o = pa_context_get_source_output_info(context, index, pac->sourceOutputCallBack, pac))) {
                qDebug("pa_context_get_source_output_info() failed");
                return;
            }
            pa_operation_unref(o);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_CLIENT:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            pac->m_clients.removeAt(index);
            emit pac->clientRemoved(index);
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
            pac->m_cards.removeAt(index);
            emit pac->cardRemoved(index);
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
        pac->m_clients.insert(i->index, *i);
        emit pac->clientAdded(i->index);
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
        pac->m_sinks.insert(i->index, *i);
        emit pac->sinkRemoved(i->index);
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
        pac->m_sources.insert(i->index, *i);
        emit pac->sourceAdded(i->index);
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
        pac->m_sinksInput.insert(i->index, *i);
        emit pac->sinkInputAdded(i->index);
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
        pac->m_sourceOutputs.insert(i->index, *i);
        emit pac->sourceOutputAdded(i->index);
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

    if(pac->m_defaultSinkName != i->default_sink_name) {
        pac->m_defaultSinkName = i->default_sink_name;
        emit pac->defaultSinkNameChanged();
    }

    if(pac->m_defaultSourceName != i->default_source_name) {
        pac->m_defaultSourceName = i->default_source_name;
        emit pac->defaultSourceNameChanged();
    }
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
        pac->m_cards.insert(i->index, *i);
        pac->cardAdded(i->index);
    }
}

void PulseAudioControl::setVolumeCallBack(pa_context *, int success, void *)
{
    if(success < 0) {
        qWarning() << "Set volume falled";
    }
}

void PulseAudioControl::setupDefaultSink()
{
    pa_context_get_sink_info_by_name(m_paContext, m_defaultSinkName.toUtf8(), setupDefaultSinkCallBack, this);
}

void PulseAudioControl::setupDefaultSinkCallBack(pa_context *, const pa_sink_info *i, int eol, void *userdata)
{
    PulseAudioControl *pac = static_cast<PulseAudioControl*>(userdata);
    if(eol < 0) {
        if (pa_context_errno(pac->m_paContext) == PA_ERR_NOENTITY) {
            return;
        }
        qWarning("Source output callback failure");
    }

    if(i == nullptr) {
        return;
    }
    pac->defaultSinkChannels = i->volume.channels;

    /*
     * if default channel mutted - unmute it
    */
    pa_context_set_sink_mute_by_name(pac->m_paContext, pac->m_defaultSinkName.toUtf8(), 0, nullptr, nullptr);
    emit pac->volumeChanged(pac->paVolume2Percent(i->volume.values[0]), pac->paVolume2Percent(PA_VOLUME_MAX));
}

void PulseAudioControl::setVolume(int volume)
{
    if(defaultSinkChannels == -1) {
        return;
    }

    pa_operation* o;

    pa_cvolume cvol;
    cvol.channels = defaultSinkChannels;

    if(percent2PaVolume(volume) > percent2PaVolume(PA_VOLUME_NORM)) {
        emit highVolume(paVolume2Percent(PA_VOLUME_NORM));
    }

    for(int i=0; i < cvol.channels; i++) {
        cvol.values[i] = percent2PaVolume(volume);
    }

    if (!(o = pa_context_set_sink_volume_by_name(m_paContext, m_defaultSinkName.toUtf8(), &cvol, setVolumeCallBack, nullptr))) {
        qWarning("pa_context_set_source_volume_by_name FAILED!");
    } else {
        pa_operation_unref(o);
    }
}
