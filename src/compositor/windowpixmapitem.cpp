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
#include "shaders/surfacenode.h"

#include <QSGMaterial>

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
    m_surfaceDestroyed = true;
    m_hasBuffer = false;
    m_item->imageRelease(this);
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

QSGNode *WindowPixmapItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    SurfaceNode *node = static_cast<SurfaceNode *>(oldNode);
    if(!m_item && !m_haveSnapshot) {
        if(node) {
            node->setTextureProvider(nullptr, false);
        }
        delete node;
        return nullptr;
    }

    QSGTextureProvider *provider = nullptr;
    QSGTexture *texture = nullptr;
    if (m_item) {
        provider = m_item->textureProvider();
        if(provider) {
            texture = provider->texture();
        }
        if(texture && !m_surfaceDestroyed) {
            m_haveSnapshot = false;
        }
    }

    if(!m_hasBuffer && texture) {
        if (!m_textureProvider) {
            SnapshotTextureProvider *prov = new SnapshotTextureProvider;
            m_textureProvider = prov;
        }

        if(!s_snapshotProgram) {

            s_snapshotProgram = new SnapshotProgram;
            s_snapshotProgram->program.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                                               "#version 300 es\n"
                                                               "layout(location = 0) in vec4 vertex;\n"
                                                               "out vec2 texPos;\n"
                                                               "void main(void) {\n"
                                                               "   texPos = vertex.xy;\n"
                                                               "   gl_Position = vec4(vertex.xy * 2.0 - 1.0, 0.0, 1.0);\n"
                                                               "}");

            s_snapshotProgram->program.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                                               "#version 300 es\n"
                                                               "precision highp float;\n"
                                                               "uniform sampler2D sourceTexture;\n"
                                                               "in vec2 texPos;\n"
                                                               "out vec4 FragColor;\n"
                                                               "void main(void) {\n"
                                                               "   FragColor = texture(sourceTexture, texPos);\n"
                                                               "}");

            if(!s_snapshotProgram->program.link()) {
                qDebug() << s_snapshotProgram->program.log();
            }

            s_snapshotProgram->vertexLocation = s_snapshotProgram->program.attributeLocation("vertex");
            s_snapshotProgram->textureLocation = s_snapshotProgram->program.uniformLocation("sourceTexture");

            connect(window(), &QQuickWindow::sceneGraphInvalidated, this, &WindowPixmapItem::cleanupOpenGL);
        }
    }

    provider = m_textureProvider;

    if(m_hasBuffer && texture) {
        SnapshotTextureProvider *prov = static_cast<SnapshotTextureProvider *>(provider);
        if(!prov) {
            qWarning() << "Wrong provider";
            return nullptr;
        } else if (!prov->fbo || prov->fbo->size() != QSize(width(), height())) {
            delete prov->t;
            prov->t = nullptr;
        }

        if (!prov->fbo || prov->fbo->size() != size().toSize()) {
            prov->fbo = new QOpenGLFramebufferObject(size().toSize());
        }

        prov->fbo->bind();
        s_snapshotProgram->program.bind();

        static GLfloat const triangleVertices[] = {
            1.f, 0.f,
            1.f, 1.f,
            0.f, 0.f,
            0.f, 1.f,
        };
        s_snapshotProgram->program.enableAttributeArray(s_snapshotProgram->vertexLocation);
        s_snapshotProgram->program.setAttributeArray(s_snapshotProgram->vertexLocation, triangleVertices, 2);

        glViewport(0, 0, width(), height());
        glDisable(GL_BLEND);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        s_snapshotProgram->program.release();

        if (!prov->t && prov->fbo) {
            prov->t = QNativeInterface::QSGOpenGLTexture::fromNative(prov->fbo->texture(), window(), prov->fbo->size());
            emit prov->textureChanged();
        }

        prov->fbo->release();
        s_snapshotProgram->program.disableAttributeArray(s_snapshotProgram->vertexLocation);

        m_haveSnapshot = true;
    } else if (!m_hasBuffer && m_textureProvider) {
        provider = m_textureProvider;
    } else if (!provider) {
        if (node)
            node->setTextureProvider(0, false);
        delete node;
        return nullptr;
    }

    if (provider != m_textureProvider) {
        delete m_textureProvider;
        m_textureProvider = nullptr;
    }

    if (m_surfaceDestroyed && m_item) {
        m_item->setDelayRemove(false);
    }

    if (!provider->texture()) {
        qWarning("WindowPixmapItem does not have a source texture, cover will be dropped..");
        if (node) {
            node->setTextureProvider(0, false);
            delete node;
        }
        return nullptr;
    }

    if (!node) node = new SurfaceNode;

    node->setTextureProvider(provider, provider == m_textureProvider);
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
    if(m_hasBuffer != hasBuffer) {
        m_hasBuffer = hasBuffer;
        if(m_hasBuffer) {
            m_item->view()->setBufferLocked(m_hasBuffer);
        }
        update();
    }
}

void WindowPixmapItem::cleanupOpenGL()
{
    disconnect(window(), &QQuickWindow::sceneGraphInvalidated, this, &WindowPixmapItem::cleanupOpenGL);
    delete s_snapshotProgram;
    s_snapshotProgram = 0;
}
