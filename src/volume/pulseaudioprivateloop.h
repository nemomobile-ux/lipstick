#ifndef PULSEAUDIOPRIVATELOOP_H
#define PULSEAUDIOPRIVATELOOP_H

#include <QThread>
#include <pulse/pulseaudio.h>

enum class AudioRole {
    Media,
    Call,
    Alarm,
    System,
    Unknow = 255,
};

class PulseAudioPrivateLoop : public QThread
{
    Q_OBJECT

public:
    explicit PulseAudioPrivateLoop(QObject *parent = nullptr);
    ~PulseAudioPrivateLoop() override;

    void setVolume(int percent);
    int currentVolume() const { return m_lastVolume; }
    void requestVolume();

    void setMute(bool muted);
    bool isMuted() const { return m_lastMuted; }

signals:
    void volumeChanged(int percent, int max);
    void muteChanged(bool muted);
    void connected();
    void disconnected();

protected:
    void run() override;

private:
    pa_mainloop *m_mainloop;
    pa_context  *m_context;
    int m_lastVolume;
    bool m_reconnecting;
    bool m_lastMuted;

    static void contextStateCallback(pa_context *c, void *userdata);
    static void sinkInfoCallback(
        pa_context *c,
        const pa_sink_info *info,
        int eol,
        void *userdata
        );

    static void subscribeCallback(
        pa_context *c,
        pa_subscription_event_type_t t,
        uint32_t idx,
        void *userdata
        );

    static void sinkInputInfoCallback(
        pa_context *,
        const pa_sink_input_info *info,
        int eol,
        void *userdata);

    void reconnect();
    void cleanupContext();
    void onContextReady();
};

#endif // PULSEAUDIOPRIVATELOOP_H
