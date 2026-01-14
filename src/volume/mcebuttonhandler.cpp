#include "mcebuttonhandler.h"
#include <QDebug>

MceButtonHandler::MceButtonHandler(QObject *parent)
    : QObject(parent)
{
    // Подписка на MCE DBus сигналы
    QDBusConnection::systemBus().connect(
        "com.meego.MCE",
        "/com/meego/MCE",
        "com.meego.MCE.Signal",
        "VolumeUp",
        this,
        SIGNAL(volumeUp)
    );

    QDBusConnection::systemBus().connect(
        "com.meego.MCE",
        "/com/meego/MCE",
        "com.meego.MCE.Signal",
        "VolumeDown",
        this,
        SIGNAL(volumeDown)
    );

    QDBusConnection::systemBus().connect(
        "com.meego.MCE",
        "/com/meego/MCE",
        "com.meego.MCE.Signal",
        "VolumeMute",
        this,
        SIGNAL(volumeMute)
    );
}
