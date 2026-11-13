#ifndef UT_WINDOWPROPERTY_H
#define UT_WINDOWPROPERTY_H

#include <QObject>

class WindowProperty;

class Ut_WindowProperty : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void testConstruction();
    void testWindowId();
    void testProperty();
    void testValueWithoutWindow();

private:
    WindowProperty *windowProperty;
};

#endif