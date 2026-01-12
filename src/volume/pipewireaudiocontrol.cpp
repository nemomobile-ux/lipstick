#include "pipewireaudiocontrol.h"
#include <spa/pod/builder.h>
#include <QTimer>

static const pw_registry_events registryEvents = {
    PW_VERSION_REGISTRY_EVENTS,
    .global = PipeWireAudioControl::onRegistryGlobal,
    .global_remove = PipeWireAudioControl::onRegistryRemove,
};
void PrivatePipewireLoop::run()
{
    pw_init(nullptr, nullptr);
    mainLoop = pw_main_loop_new(nullptr);
    emit pipewireReady();
    pw_main_loop_run(mainLoop);
    pw_main_loop_destroy(mainLoop);
    pw_deinit();
}

// ------------------ PipeWireAudioControl ------------------

PipeWireAudioControl::PipeWireAudioControl(QObject *parent)
    : QObject(parent)
{
    connect(&m_loopThread,
            &PrivatePipewireLoop::pipewireReady,
            this,
            &PipeWireAudioControl::initPipeWire,
            Qt::QueuedConnection);

    m_loopThread.start();
}

PipeWireAudioControl::~PipeWireAudioControl()
{
    if (m_core)
        pw_core_disconnect(m_core);
    if (m_context)
        pw_context_destroy(m_context);
    pw_deinit();
}

void PipeWireAudioControl::initPipeWire()
{
    if (!m_loopThread.mainLoop) {
        qWarning() << "PipeWire main loop not initialized";
        return;
    }

    m_context = pw_context_new(
        pw_main_loop_get_loop(m_loopThread.mainLoop)
        , nullptr
        , 0);

    if (!m_context) {
        qWarning() << "Failed to create PipeWire context";
        return;
    }

    m_core = pw_context_connect(m_context, nullptr, 0);
    if (!m_core) {
        qWarning() << "Failed to connect to PipeWire core";
        return;
    }

    m_registry = pw_core_get_registry(m_core, PW_VERSION_REGISTRY, 0);
    if (!m_registry) {
        qWarning() << "Failed to get PipeWire registry";
        return;
    }

    pw_registry_add_listener(m_registry
                             , &m_registryListener
                             , &registryEvents
                             , this);
}

void PipeWireAudioControl::onRegistryGlobal(void *data
                                            , uint32_t id
                                            , uint32_t permissions
                                            , const char *type
                                            , uint32_t version
                                            , const spa_dict *props)
{
    auto *self = static_cast<PipeWireAudioControl*>(data);
    if (strcmp(type, PW_TYPE_INTERFACE_Node) == 0)
        self->handleNode(id, props);
}

void PipeWireAudioControl::onRegistryRemove(void *data, uint32_t id)
{
    auto *self = static_cast<PipeWireAudioControl*>(data);
    if (id == self->m_defaultSinkId) {
        self->m_defaultSinkId = SPA_ID_INVALID;
        self->m_defaultSinkPriority = -1;
        qDebug() << "Default sink removed, recalculating...";
    }
}

void PipeWireAudioControl::handleNode(uint32_t id, const spa_dict *props)
{
    const char *mediaClass = spa_dict_lookup(props, "media.class");
    if (!mediaClass || strcmp(mediaClass, "Audio/Sink") != 0)
        return;

    const char *priorityStr = spa_dict_lookup(props, "priority.session");
    int priority = priorityStr ? atoi(priorityStr) : 0;

    const char *nodeName = spa_dict_lookup(props, "node.name");
    if (priority > m_defaultSinkPriority) {
        m_defaultSinkPriority = priority;
        m_defaultSinkId = id;

        qDebug() << "Default sink selected:" << nodeName;
    }
}


void PipeWireAudioControl::update()
{

}

void PipeWireAudioControl::setVolume(int volume)
{
    if (m_defaultSinkId == SPA_ID_INVALID) {
        qWarning() << "Default sink/route not ready";
        return;
    }

    float vol = qBound(0, volume, 150) / 100.0f;

    uint8_t buffer[256];
    spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

    const spa_pod *param = (spa_pod*) spa_pod_builder_add_object(
        &builder,
        SPA_TYPE_OBJECT_Props,
        SPA_PARAM_Props,
        0, // volume
        SPA_POD_Float(vol)
        );

    pw_node *node = reinterpret_cast<pw_node*>(
        pw_registry_get_object(m_registry, m_defaultSinkId)
        );

    if (!node) {
        qWarning() << "Default sink node not found";
        return;
    }

    pw_node_set_param(node, SPA_PARAM_Props, 0, param);
    qDebug() << "Node volume set to" << percent << "%";
