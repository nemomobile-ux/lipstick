#ifndef UT_LAUNCHERFOLDER_H
#define UT_LAUNCHERFOLDER_H

#include <QObject>

class Ut_LauncherFolder : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void loadDirectoryFileReadsIcon();
    void saveDirectoryFileWritesIcon();
    void saveDirectoryFilePreservesOtherKeys();
};

#endif
