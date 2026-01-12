#include "pulseaudioprivateloop.h"
#include <QDebug>

PulseAudioPrivateLoop::PulseAudioPrivateLoop(QObject *parent)
    : QThread(parent)
    , m_mainloop(nullptr)
    , m_context(nullptr)
    , m_lastVolume(-1)
    , m_reconnecting(false)
    , m_lastMuted(false)
{
}

PulseAudioPrivateLoop::~PulseAudioPrivateLoop()
{
    if (m_context) {
        pa_context_disconnect(m_context);
        pa_context_unref(m_context);
    }

    if (m_mainloop) {
        pa_mainloop_quit(m_mainloop, 0);
        pa_mainloop_free(m_mainloop);
    }

    wait();
}

void PulseAudioPrivateLoop::run()
{
    m_mainloop = pa_mainloop_new();
    pa_mainloop_api* api = pa_mainloop_get_api(m_mainloop);

    m_context = pa_context_new(api, "lipstick");
    pa_context_set_state_callback(m_context, contextStateCallback, this);

    pa_context_connect(m_context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);

    int ret;
    pa_mainloop_run(m_mainloop, &ret);
}

void PulseAudioPrivateLoop::contextStateCallback(pa_context *c, void *userdata)
{
    PulseAudioPrivateLoop* self = static_cast<PulseAudioPrivateLoop*>(userdata);

    switch (pa_context_get_state(c)) {
    case PA_CONTEXT_READY:
        self->m_reconnecting = false;
        self->onContextReady();
        emit self->connected();
        break;

    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
        if (!self->m_reconnecting) {
            self->m_reconnecting = true;
            emit self->disconnected();
            self->reconnect();
        }
        break;

    default:
        break;
    }
}

void PulseAudioPrivateLoop::onContextReady()
{
    pa_context_set_subscribe_callback(
        m_context,
        subscribeCallback,
        this
    );

    pa_context_subscribe(
        m_context,
        static_cast<pa_subscription_mask_t>(PA_SUBSCRIPTION_MASK_SINK),
        nullptr,
        nullptr
    );

    requestVolume();
}

void PulseAudioPrivateLoop::requestVolume()
{
    if (!m_context) {
        return;
    }

    pa_context_get_sink_info_by_name(
        m_context,
        "@DEFAULT_SINK@",
        sinkInfoCallback,
        this
        );
}

void PulseAudioPrivateLoop::setMute(bool muted)
{
    if (!m_context) {
        return;
    }

    m_lastMuted = muted;

    pa_context_set_sink_mute_by_name(
        m_context,
        "@DEFAULT_SINK@",
        muted,
        nullptr,
        nullptr
    );
}

void PulseAudioPrivateLoop::sinkInfoCallback(
    pa_context *,
    const pa_sink_info *info,
    int eol,
    void *userdata)
{
    if (eol || !info) {
        return;
    }

    auto self = static_cast<PulseAudioPrivateLoop*>(userdata);

    int volume = static_cast<int>(
            pa_cvolume_avg(&info->volume) * 100 / PA_VOLUME_NORM
            );

    if (volume != self->m_lastVolume) {
        self->m_lastVolume = volume;
        emit self->volumeChanged(volume, 100);
    }

    bool muted = info->mute;

    if (muted != self->m_lastMuted) {
        self->m_lastMuted = muted;
        emit self->muteChanged(muted);
    }
}


void PulseAudioPrivateLoop::subscribeCallback(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata)
{
    PulseAudioPrivateLoop* self = static_cast<PulseAudioPrivateLoop*>(userdata);

    pa_subscription_event_type_t type =
        static_cast<pa_subscription_event_type_t>(
            t & PA_SUBSCRIPTION_EVENT_TYPE_MASK
            );

    pa_subscription_event_type_t facility =
        static_cast<pa_subscription_event_type_t>(
            t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK
            );

    if (facility == PA_SUBSCRIPTION_EVENT_SINK &&
        (type == PA_SUBSCRIPTION_EVENT_CHANGE ||
         type == PA_SUBSCRIPTION_EVENT_NEW)) {

        self->requestVolume();
    }
}

void PulseAudioPrivateLoop::reconnect()
{
    cleanupContext();

    pa_usec_t delay = 500 * PA_USEC_PER_MSEC;
    pa_usec_t target = pa_rtclock_now() + delay;

    struct timeval tv;
    pa_timeval_store(&tv, target);

    pa_mainloop_api *api = pa_mainloop_get_api(m_mainloop);

    api->time_new(
        api,
        &tv,
        [](pa_mainloop_api *api,
           pa_time_event *e,
           const struct timeval *,
           void *userdata)
        {
            auto self = static_cast<PulseAudioPrivateLoop*>(userdata);

            api->time_free(e);

            self->m_context = pa_context_new(
                pa_mainloop_get_api(self->m_mainloop),
                "QtPulseAudio"
                );

            pa_context_set_state_callback(
                self->m_context,
                contextStateCallback,
                self
                );

            pa_context_connect(
                self->m_context,
                nullptr,
                PA_CONTEXT_NOFLAGS,
                nullptr
                );
        },
        this
    );
}


void PulseAudioPrivateLoop::cleanupContext()
{
    if (!m_context) {
        return;
    }

    pa_context_disconnect(m_context);
    pa_context_unref(m_context);
    m_context = nullptr;

    m_lastVolume = -1;
}

void PulseAudioPrivateLoop::setVolume(int percent)
{
    if (!m_context) {
        return;
    }

    m_lastVolume = percent;
    pa_cvolume volume;
    pa_cvolume_set(
        &volume,
        2,
        pa_sw_volume_from_linear(m_lastVolume / 100.0)
        );

    pa_context_set_sink_volume_by_name(
        m_context,
        "@DEFAULT_SINK@",
        &volume,
        nullptr,
        nullptr
    );
}
