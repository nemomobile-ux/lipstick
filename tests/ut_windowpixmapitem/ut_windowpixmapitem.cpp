#include <QtTest/QtTest>
#include <QSignalSpy>

#include "windowpixmapitem.h"
#include "ut_windowpixmapitem.h"
#include "lipsticktest.h"

void Ut_WindowPixmapItem::init()
{
    item = new WindowPixmapItem();
}

void Ut_WindowPixmapItem::cleanup()
{
    delete item;
    item = nullptr;
}

void Ut_WindowPixmapItem::testConstruction()
{
    QVERIFY(item != nullptr);
    QCOMPARE(item->windowId(), 0);
    QVERIFY(!item->hasPixmap());
    QCOMPARE(item->opaque(), false);
    QCOMPARE(item->radius(), 0.0);
    QCOMPARE(item->xOffset(), 0.0);
    QCOMPARE(item->yOffset(), 0.0);
    QCOMPARE(item->xScale(), 1.0);
    QCOMPARE(item->yScale(), 1.0);
}

void Ut_WindowPixmapItem::testWindowId()
{
    QSignalSpy spy(item, &WindowPixmapItem::windowIdChanged);
    item->setWindowId(7);
    QCOMPARE(item->windowId(), 7);
    QCOMPARE(spy.count(), 1);

    item->setWindowId(7);
    QCOMPARE(spy.count(), 1);
}

void Ut_WindowPixmapItem::testProperties()
{
    QSignalSpy opaqueSpy(item, &WindowPixmapItem::opaqueChanged);
    item->setOpaque(true);
    QCOMPARE(item->opaque(), true);
    QCOMPARE(opaqueSpy.count(), 1);

    QSignalSpy radiusSpy(item, &WindowPixmapItem::radiusChanged);
    item->setRadius(5.0);
    QCOMPARE(item->radius(), 5.0);
    QCOMPARE(radiusSpy.count(), 1);

    QSignalSpy xScaleSpy(item, &WindowPixmapItem::xScaleChanged);
    item->setXScale(2.0);
    QCOMPARE(item->xScale(), 2.0);
    QCOMPARE(xScaleSpy.count(), 1);
}

LIPSTICK_TEST_MAIN(Ut_WindowPixmapItem)