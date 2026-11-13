#ifndef UT_WINDOWPIXMAPITEM_H
#define UT_WINDOWPIXMAPITEM_H

#include <QObject>

class WindowPixmapItem;

class Ut_WindowPixmapItem : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void testConstruction();
    void testWindowId();
    void testProperties();

private:
    WindowPixmapItem *item;
};

#endif