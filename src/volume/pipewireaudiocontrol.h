#ifndef PIPEWIREAUDIOCONTROL_H
#define PIPEWIREAUDIOCONTROL_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <pipewire/pipewire.h>

class PrivatePipewireLoop : public QThread
{
    Q_OBJECT
public:
    struct pw_main_loop *mainLoop = nullptr;
signals:
    void pipewireReady();
protected:
    void run() override;
};

class PipeWireAudioControl : public QObject
{
    Q_OBJECT
public:
    explicit PipeWireAudioControl(QObject *parent = nullptr);
    ~PipeWireAudioControl();

    static void onRegistryGlobal(
        void *data,
        uint32_t id,
        uint32_t permissions,
        const char *type,
        uint32_t version,
        const spa_dict *props
    );

    static void onRegistryRemove(void *data, uint32_t id);

public slots:
    void update();
    void setVolume(int volume);

signals:
    void longListeningTime(int);
    void callActiveChanged(bool);
    void mediaStateChanged(QString);

    void volumeChanged(int volume, int maximum);
    void highVolume(int volume);

private:
    void initPipeWire();
    void handleNode(uint32_t id, const spa_dict *props);

    pw_context *m_context = nullptr;
    pw_core *m_core = nullptr;
    pw_registry *m_registry = nullptr;
    spa_hook m_registryListener{};

    uint32_t m_defaultSinkId    = SPA_ID_INVALID;
    uint32_t m_defaultDeviceId  = SPA_ID_INVALID;
    uint32_t m_defaultRouteIndex= 0;
    int m_defaultSinkPriority   = -1;

    PrivatePipewireLoop m_loopThread;
};

#endif // PIPEWIREAUDIOCONTROL_H
