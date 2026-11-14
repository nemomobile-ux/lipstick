#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    engine.addImportPath(QCoreApplication::applicationDirPath());
    engine.addImportPath(QCoreApplication::applicationDirPath() + "/../plugin");
    engine.addImportPath(QCoreApplication::applicationDirPath() + "/../../plugin");
    engine.addImportPath(QCoreApplication::applicationDirPath() + "/../src");

    QString qmlFile;
    int windowId = 0;

    if (argc > 1 && QString::fromLocal8Bit(argv[1]).endsWith(QStringLiteral(".qml"))) {
        qmlFile = argv[1];
        if (argc > 2)
            windowId = QString::fromLocal8Bit(argv[2]).toInt();
    } else if (argc > 1) {
        windowId = QString::fromLocal8Bit(argv[1]).toInt();
    }

    if (qEnvironmentVariableIsSet("WINDOW_ID"))
        windowId = qgetenv("WINDOW_ID").toInt();

    if (qmlFile.isEmpty()) {
        qmlFile = !QStringLiteral(QML_FILE_PATH).isEmpty() && QFileInfo(QStringLiteral(QML_FILE_PATH)).exists()
                      ? QStringLiteral(QML_FILE_PATH)
                      : QStringLiteral("windowpixmapitem.qml");
    }

    engine.rootContext()->setContextProperty(QStringLiteral("windowIdOverride"), windowId);
    engine.load(QUrl::fromLocalFile(QFileInfo(qmlFile).absoluteFilePath()));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}