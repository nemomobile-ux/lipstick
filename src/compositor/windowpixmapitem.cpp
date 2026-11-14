/***************************************************************************
**
** Copyright (c) 2013 Jolla Ltd.
**
** This file is part of lipstick.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#include "windowpixmapitem.h"
#include "lipstickcompositor.h"
#include "lipstickcompositorwindow.h"
#include "shaders/roundedclipmaterial.h"
#include "shaders/surfacenode.h"

#include <QQmlProperty>
#include <QSGGeometryNode>
#include <QSGMaterial>
#include <QSGSimpleTextureNode>
#include <QSGTexture>
#include <QtMath>

SnapshotProgram *WindowPixmapItem::s_snapshotProgram = nullptr;

WindowPixmapItem::WindowPixmapItem()
    : m_item(0)
    , m_id(0)
    , m_opaque(false)
    , m_radius(0)
    , m_xOffset(0)
    , m_yOffset(0)
    , m_xScale(1)
    , m_yScale(1)
    , m_hasBuffer(false)
    , m_hasPixmap(false)
    , m_surfaceDestroyed(false)
    , m_haveSnapshot(false)
    , m_textureProvider(nullptr)
{
    setFlag(ItemHasContents, true);
    setEnabled(false);
}

WindowPixmapItem::~WindowPixmapItem()
{
    setWindowId(0);
}

int WindowPixmapItem::windowId() const
{
    return m_id;
}

void WindowPixmapItem::setWindowId(int id)
{
    if (m_id == id)
        return;

    QSize oldSize = windowSize();
    if (m_item) {
        if (QSGTextureProvider *tp = m_item->textureProvider()) {
            disconnect(tp, &QSGTextureProvider::textureChanged, this, &QQuickItem::update);
        }
        QQmlProperty(m_item, QStringLiteral("layer.enabled")).write(false);
        disconnect(m_item.data(), &QObject::destroyed, this, &WindowPixmapItem::itemDestroyed);
        if(m_item->surface()) {
            disconnect(m_item->surface(), &QWaylandSurface::destinationSizeChanged, this, &WindowPixmapItem::handleWindowSizeChanged);
            disconnect(m_item->surface(), &QWaylandSurface::configure, this, &WindowPixmapItem::configure);
            disconnect(m_item->surface(), &QWaylandSurface::surfaceDestroyed, this, &WindowPixmapItem::surfaceDestroyed);
        }

        if (!m_surfaceDestroyed)
            m_item->imageRelease(this);
        m_item->setDelayRemove(false);
        m_item = nullptr;
    }

    m_surfaceDestroyed = false;
    m_hasBuffer = false;
    m_id = id;
    updateItem();
    emit windowIdChanged();

    if(windowSize() != oldSize) {
         emit windowSizeChanged();
    }
}

void WindowPixmapItem::surfaceDestroyed()
{
    if (m_surfaceDestroyed)
        return;
    m_surfaceDestroyed = true;
    m_hasBuffer = false;
    if (m_item) {
        m_item->imageRelease(this);
    }
    update();
}

bool WindowPixmapItem::hasPixmap() const
{
    return m_hasPixmap;
}

bool WindowPixmapItem::opaque() const
{
    return m_opaque;
}

void WindowPixmapItem::setOpaque(bool o)
{
    if (m_opaque == o)
        return;

    m_opaque = o;
    if (m_item) update();

    emit opaqueChanged();
}

qreal WindowPixmapItem::radius() const
{
    return m_radius;
}

void WindowPixmapItem::setRadius(qreal r)
{
    if (m_radius == r)
        return;

    m_radius = r;
    if (m_item) update();

    emit radiusChanged();
}

qreal WindowPixmapItem::xOffset() const
{
    return m_xOffset;
}

void WindowPixmapItem::setXOffset(qreal xOffset)
{
    if (m_xOffset == xOffset)
        return;

    m_xOffset = xOffset;
    if (m_item) update();

    emit xOffsetChanged();
}

qreal WindowPixmapItem::yOffset() const
{
    return m_yOffset;
}

void WindowPixmapItem::setYOffset(qreal yOffset)
{
    if (m_yOffset == yOffset)
        return;

    m_yOffset = yOffset;
    if (m_item) update();

    emit yOffsetChanged();
}

qreal WindowPixmapItem::xScale() const
{
    return m_xScale;
}

void WindowPixmapItem::setXScale(qreal xScale)
{
    if (m_xScale == xScale)
        return;

    m_xScale = xScale;
    if (m_item || m_haveSnapshot) update();

    emit xScaleChanged();
}

qreal WindowPixmapItem::yScale() const
{
    return m_yScale;
}

void WindowPixmapItem::setYScale(qreal yScale)
{
    if (m_yScale == yScale)
        return;

    m_yScale = yScale;
    if (m_item || m_haveSnapshot) update();

    emit yScaleChanged();
}

QSize WindowPixmapItem::windowSize() const
{
    return m_windowSize;
}

void WindowPixmapItem::setWindowSize(const QSize &s)
{
    if (!m_item || !m_item->surface()) {
        return;
    }

    //TODO m_item->surface()->requestSize(s);
}

void WindowPixmapItem::handleWindowSizeChanged()
{
    if(m_item->surface()->destinationSize().isValid()) {
        m_windowSize = m_item->surface()->destinationSize();
        emit windowSizeChanged();
    }
}

void WindowPixmapItem::itemDestroyed(QObject *)
{
    m_item = nullptr;
    if (!m_haveSnapshot) {
        m_hasPixmap = false;
        emit hasPixmapChanged();
    }
}

/* rounded clip geometry helper removed - using shader material */

QSGNode *WindowPixmapItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QSGTextureProvider *liveProvider = nullptr;
    QSGTexture *liveTexture = nullptr;
    if (m_item) {
        liveProvider = m_item->textureProvider();
        if (!liveProvider) {
            if (!m_item->isTextureProvider()) {
                QQmlProperty(m_item, QStringLiteral("layer.enabled")).write(true);
            }
            liveProvider = m_item->textureProvider();
        }
        if (liveProvider)
            liveTexture = liveProvider->texture();
    }

    if (!liveTexture || m_surfaceDestroyed || width() <= 0 || height() <= 0) {
        delete oldNode;
        return nullptr;
    }

    const QRectF rect(0, 0, width(), height());
    const QRectF sourceRect(m_xOffset, 1.0 - m_yOffset, m_xScale, -m_yScale);

    // Rounded corners: use a shader material that applies a rounded-rect alpha
    // mask so corners are rendered transparent (instead of stencil clipping).
    if (m_radius > 0) {
        QSGGeometryNode *node = dynamic_cast<QSGGeometryNode *>(oldNode);
        RoundedClipMaterial *mat = nullptr;
        if (!node || !dynamic_cast<RoundedClipMaterial *>(node->material())) {
            delete oldNode;
            node = new QSGGeometryNode();
            QSGGeometry *geom = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);
            node->setGeometry(geom);
            node->setFlag(QSGNode::OwnsGeometry);
            mat = new RoundedClipMaterial();
            node->setMaterial(mat);
            node->setFlag(QSGNode::OwnsMaterial);
        } else {
            mat = static_cast<RoundedClipMaterial *>(node->material());
        }

        QSGGeometry::updateTexturedRectGeometry(node->geometry(), rect, sourceRect);
        mat->setTexture(liveTexture);
        mat->setRadius(float(m_radius));
        mat->setItemSize(rect.size());
        node->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
        return node;
    }

    // Simple path: no rounded corners.
    {
        QSGSimpleTextureNode *n = dynamic_cast<QSGSimpleTextureNode *>(oldNode);
        if (!n) {
            delete oldNode;
            n = new QSGSimpleTextureNode();
        }
        n->setTexture(liveTexture);
        n->setRect(rect);
        n->setSourceRect(QRectF(m_xOffset * liveTexture->textureSize().width(),
                                m_yOffset * liveTexture->textureSize().height(),
                                m_xScale * liveTexture->textureSize().width(),
                                m_yScale * liveTexture->textureSize().height()));
        n->setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
        n->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);
        n->setOwnsTexture(false);
        n->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
        return n;
    }

    // Fallback to shader-based node for advanced effects.
    SurfaceNode *node = dynamic_cast<SurfaceNode *>(oldNode);
    if (!node) {
        delete oldNode;
        node = new SurfaceNode();
    }

    node->setTextureProvider(liveProvider, false);
    node->setRect(QRectF(0, 0, width(), height()));
    node->setBlending(!m_opaque);
    node->setRadius(m_radius);
    node->setXOffset(m_xOffset);
    node->setYOffset(m_yOffset);
    node->setXScale(m_xScale);
    node->setYScale(m_yScale);
    node->updateGeometry();
    return node;
}


void WindowPixmapItem::updateItem()
{
    LipstickCompositor *c = LipstickCompositor::instance();
    Q_ASSERT(m_item == 0);

    if (c && m_id) {
        LipstickCompositorWindow *w = static_cast<LipstickCompositorWindow *>(c->windowForId(m_id));
        if (!w) {
            qWarning() << "WindowPixmapItem: no window for id" << m_id;
            if (m_hasPixmap && !m_haveSnapshot) {
                m_hasPixmap = false;
                emit hasPixmapChanged();
            }
            return;
        } else if (w->surface()) {
            m_item = w;
            m_item->setDelayRemove(true);
            connect(m_item->surface(), &QWaylandSurface::destinationSizeChanged, this, &WindowPixmapItem::handleWindowSizeChanged);
            connect(m_item->surface(), &QWaylandSurface::configure, this, &WindowPixmapItem::configure);
            connect(m_item.data(), &QWaylandQuickItem::surfaceDestroyed, this, &WindowPixmapItem::surfaceDestroyed);
            connect(m_item.data(), &QObject::destroyed, this, &WindowPixmapItem::itemDestroyed);
            m_windowSize = m_item->surface()->destinationSize();
            m_hasBuffer = m_item->surface()->hasContent();

            // Enable layer so the source item can provide a texture.
            QQmlProperty(m_item, QStringLiteral("layer.enabled")).write(true);
            // Texture provider becomes available on the render thread; schedule repaint.
            QMetaObject::invokeMethod(this, &QQuickItem::update, Qt::QueuedConnection);
        } else {
            qWarning() << "WindowPixmapItem: window" << m_id << "has no surface";
        }

        w->imageAddref(this);

        update();
    }
    const bool hadPixmap = m_hasPixmap;
    m_hasPixmap = m_item || m_haveSnapshot;
    if (m_hasPixmap != hadPixmap) {
        emit hasPixmapChanged();
    }
}

void WindowPixmapItem::configure(bool hasBuffer)
{
    if (m_hasBuffer != hasBuffer) {
        m_hasBuffer = hasBuffer;
        update();
    }
}

void WindowPixmapItem::cleanupOpenGL()
{
    disconnect(window(), &QQuickWindow::sceneGraphInvalidated, this, &WindowPixmapItem::cleanupOpenGL);
    delete s_snapshotProgram;
    s_snapshotProgram = 0;
}
