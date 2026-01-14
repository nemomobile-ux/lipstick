#ifndef MCEBUTTONHANDLER_H
#define MCEBUTTONHANDLER_H
#include <QObject>
#include <QDBusConnection>
#include <QTimer>

class MceButtonHandler : public QObject
{
    Q_OBJECT
public:
    explicit MceButtonHandler(QObject *parent = nullptr);

signals:
    void volumeUp();
    void volumeDown();
    void volumeMute();
};


#endif // MCEBUTTONHANDLER_H
