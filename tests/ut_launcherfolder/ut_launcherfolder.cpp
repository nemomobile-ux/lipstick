#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QTemporaryDir>

#include "launcherfoldermodel.h"
#include "mdesktopentry.h"

#include "ut_launcherfolder.h"

// Minimal MDesktopEntry stubs (launcheritem/launchermodel are compiled into this test)
class MDesktopEntryPrivate
{
public:
    explicit MDesktopEntryPrivate(const QString &fileName) : m_fileName(fileName) {}
    QString m_fileName;
};

MDesktopEntry::MDesktopEntry(const QString &fileName)
    : d_ptr(new MDesktopEntryPrivate(fileName))
{
}

MDesktopEntry::~MDesktopEntry() { delete d_ptr; }
QString MDesktopEntry::fileName() const { return d_ptr->m_fileName; }
QString MDesktopEntry::exec() const { return QString(); }
QString MDesktopEntry::name() const { return QString(); }
QString MDesktopEntry::icon() const { return QString(); }
QString MDesktopEntry::type() const { return QStringLiteral("Application"); }
QStringList MDesktopEntry::categories() const { return QStringList(); }
QStringList MDesktopEntry::mimeType() const { return QStringList(); }
QString MDesktopEntry::nameUnlocalized() const { return QString(); }
QString MDesktopEntry::url() const { return QString(); }
QString MDesktopEntry::xMaemoService() const { return QString(); }
bool MDesktopEntry::noDisplay() const { return false; }
QStringList MDesktopEntry::notShowIn() const { return QStringList(); }
bool MDesktopEntry::isValid() const { return true; }
uint MDesktopEntry::hash() const { return 0; }
QString MDesktopEntry::value(const QString &, const QString &) const { return QString(); }

static QTemporaryDir g_tempDir;

void Ut_LauncherFolder::initTestCase()
{
    QVERIFY(g_tempDir.isValid());
    LauncherFolderModel::setConfigDir(g_tempDir.path() + QLatin1Char('/'));
}

void Ut_LauncherFolder::cleanupTestCase()
{
}

void Ut_LauncherFolder::loadDirectoryFileReadsIcon()
{
    const QString path = g_tempDir.path() + QStringLiteral("/read.directory");
    {
        QSettings settings(path, QSettings::IniFormat);
        settings.beginGroup(QStringLiteral("Desktop Entry"));
        settings.setValue(QStringLiteral("Type"), QStringLiteral("Directory"));
        settings.setValue(QStringLiteral("Name"), QStringLiteral("Test"));
        settings.setValue(QStringLiteral("Icon"), QStringLiteral("icon-from-file"));
        settings.endGroup();
        settings.sync();
    }

    LauncherFolderItem folder;
    folder.loadDirectoryFile(path);

    QCOMPARE(folder.iconId(), QStringLiteral("icon-from-file"));
    QCOMPARE(folder.directoryFile(), path);
}

void Ut_LauncherFolder::saveDirectoryFileWritesIcon()
{
    const QString path = g_tempDir.path() + QStringLiteral("/write.directory");
    // Seed with an existing file so saveDirectoryFile writes to it
    {
        QSettings settings(path, QSettings::IniFormat);
        settings.beginGroup(QStringLiteral("Desktop Entry"));
        settings.setValue(QStringLiteral("Type"), QStringLiteral("Directory"));
        settings.setValue(QStringLiteral("Name"), QStringLiteral("Test"));
        settings.endGroup();
        settings.sync();
    }

    LauncherFolderItem folder;
    folder.loadDirectoryFile(path);
    folder.setIconId(QStringLiteral("icon-updated"));

    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("Desktop Entry"));
    QCOMPARE(settings.value(QStringLiteral("Icon")).toString(), QStringLiteral("icon-updated"));
    settings.endGroup();
}

void Ut_LauncherFolder::saveDirectoryFilePreservesOtherKeys()
{
    const QString path = g_tempDir.path() + QStringLiteral("/preserve.directory");
    {
        QSettings settings(path, QSettings::IniFormat);
        settings.beginGroup(QStringLiteral("Desktop Entry"));
        settings.setValue(QStringLiteral("Type"), QStringLiteral("Directory"));
        settings.setValue(QStringLiteral("Name"), QStringLiteral("My Folder"));
        settings.setValue(QStringLiteral("Icon"), QStringLiteral("old-icon"));
        settings.setValue(QStringLiteral("Comment"), QStringLiteral("keep-me"));
        settings.endGroup();
        settings.sync();
    }

    LauncherFolderItem folder;
    folder.loadDirectoryFile(path);
    folder.setIconId(QStringLiteral("new-icon"));

    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("Desktop Entry"));
    QCOMPARE(settings.value(QStringLiteral("Icon")).toString(), QStringLiteral("new-icon"));
    QCOMPARE(settings.value(QStringLiteral("Name")).toString(), QStringLiteral("My Folder"));
    QCOMPARE(settings.value(QStringLiteral("Comment")).toString(), QStringLiteral("keep-me"));
    settings.endGroup();
}

QTEST_APPLESS_MAIN(Ut_LauncherFolder)
