#include <QtTest/QtTest>
#include <QSignalSpy>

#include "windowproperty.h"
#include "ut_windowproperty.h"
#include "lipsticktest.h"

void Ut_WindowProperty::init()
{
    windowProperty = new WindowProperty();
}

void Ut_WindowProperty::cleanup()
{
    delete windowProperty;
    windowProperty = nullptr;
}

void Ut_WindowProperty::testConstruction()
{
    QVERIFY(windowProperty != nullptr);
    QCOMPARE(windowProperty->windowId(), 0);
    QCOMPARE(windowProperty->property(), QString());
    QVERIFY(!windowProperty->value().isValid());
}

void Ut_WindowProperty::testWindowId()
{
    QSignalSpy spy(windowProperty, &WindowProperty::windowIdChanged);
    windowProperty->setWindowId(42);
    QCOMPARE(windowProperty->windowId(), 42);
    QCOMPARE(spy.count(), 1);

    // setting same value does not emit
    windowProperty->setWindowId(42);
    QCOMPARE(spy.count(), 1);
}

void Ut_WindowProperty::testProperty()
{
    QSignalSpy propertySpy(windowProperty, &WindowProperty::propertyChanged);
    QSignalSpy valueSpy(windowProperty, &WindowProperty::valueChanged);

    windowProperty->setProperty("foo");
    QCOMPARE(windowProperty->property(), QString("foo"));
    QCOMPARE(propertySpy.count(), 1);
    QVERIFY(valueSpy.count() >= 1);

    windowProperty->setProperty("foo");
    QCOMPARE(propertySpy.count(), 1);
}

void Ut_WindowProperty::testValueWithoutWindow()
{
    windowProperty->setProperty("bar");
    QVERIFY(!windowProperty->value().isValid());
}

LIPSTICK_TEST_MAIN(Ut_WindowProperty)