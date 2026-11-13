#include "surfacenode.h"

SurfaceNode::SurfaceNode()
{
    setGeometry(&m_geometry);
    setMaterial(&m_material);
    setOpaqueMaterial(&m_opaqueMaterial);

    m_cornerNode.setGeometry(&m_cornerGeometry);
    m_cornerNode.setMaterial(&m_cornerMaterial);

    m_material.setFlag(QSGMaterial::Blending, true);
    m_cornerMaterial.setFlag(QSGMaterial::Blending, true);

    m_cornerGeometry.setDrawingMode(GL_TRIANGLES);
}

SurfaceNode::~SurfaceNode()
{
    if (m_provider && m_providerOwned)
        delete m_provider;
}

void SurfaceNode::setRect(const QRectF &r)
{
    m_geometryChanged |= m_rect != r;

    m_rect = r;
}

void SurfaceNode::setTextureProvider(QSGTextureProvider *p, bool owned)
{
    if (p == nullptr || p == m_provider) {
        return;
    }

    if (m_provider) {
        QObject::disconnect(m_provider, &QSGTextureProvider::destroyed, this, &SurfaceNode::providerDestroyed);
        QObject::disconnect(m_provider, &QSGTextureProvider::textureChanged, this, &SurfaceNode::textureChanged);
        if(m_providerOwned) {
            delete m_provider;
        }
        m_provider = nullptr;
    }

    m_provider = p;
    m_providerOwned = owned;

    if (!m_provider.isNull()) {
        QObject::connect(m_provider, &QSGTextureProvider::destroyed, this, &SurfaceNode::providerDestroyed);
        QObject::connect(m_provider, &QSGTextureProvider::textureChanged, this, &SurfaceNode::textureChanged);

        setTexture(m_provider->texture());
    } else {
        qWarning() << "Provider is null!!!";
    }
}

void SurfaceNode::updateGeometry()
{
    if (m_geometryChanged && m_texture) {
        m_geometryChanged = false;

        const QSize ts = m_texture->textureSize();
        const QRectF textureRect = m_texture->convertToNormalizedSourceRect(QRectF(
            ts.width() * m_xOffset,
            ts.height() * m_yOffset,
            ts.width() * m_xScale,
            ts.height() * m_yScale));

        if (m_radius) {
            qreal radius = std::min({ m_rect.width() / 2, m_rect.height() / 2, m_radius });

            m_geometry.allocate(8);
            m_cornerGeometry.allocate(12);

            const float outerL = m_rect.left();
            const float innerL = m_rect.left() + radius;
            const float innerR = m_rect.right() - radius;
            const float outerR = m_rect.right();

            const float outerT = m_rect.top();
            const float innerT = m_rect.top() + radius;
            const float innerB = m_rect.bottom() - radius;
            const float outerB = m_rect.bottom();

            float textureXRadius = radius * textureRect.width() / m_rect.width();
            float textureYRadius = radius * textureRect.height() / m_rect.height();

            const float outerTL = textureRect.left();
            const float innerTL = textureRect.left() + textureXRadius;
            const float innerTR = textureRect.right() - textureXRadius;
            const float outerTR = textureRect.right();

            const float outerTT = textureRect.top();
            const float innerTT = textureRect.top() + textureYRadius;
            const float innerTB = textureRect.bottom() - textureYRadius;
            const float outerTB = textureRect.bottom();

            // Item rectangle with the corners clipped
            QSGGeometry::TexturedPoint2D *vertices = m_geometry.vertexDataAsTexturedPoint2D();

            vertices[0].set(outerL, innerB, outerTL, innerTB); // Outer left, inner bottom
            vertices[1].set(outerL, innerT, outerTL, innerTT); // Outer left, inner top
            vertices[2].set(innerL, outerB, innerTL, outerTB); // Inner left, outer bottom
            vertices[3].set(innerL, outerT, innerTL, outerTT); // Inner left, outer top
            vertices[4].set(innerR, outerB, innerTR, outerTB); // Inner right, outer botton
            vertices[5].set(innerR, outerT, innerTR, outerTT); // Inner right, outer top
            vertices[6].set(outerR, innerB, outerTR, innerTB); // Outer right, inner bottom
            vertices[7].set(outerR, innerT, outerTR, innerTT); // Outer right, inner top

            // Corners
            CornerVertex *corners = static_cast<CornerVertex *>(m_cornerGeometry.vertexData());

            // Bottom left
            corners[0].set(outerL, outerB, outerTL, outerTB, radius, radius);
            corners[1].set(outerL, innerB, outerTL, innerTB, radius, 0);
            corners[2].set(innerL, outerB, innerTL, outerTB, 0, radius);

            // Top left
            corners[3].set(outerL, outerT, outerTL, outerTT, radius, radius);
            corners[4].set(outerL, innerT, outerTL, innerTT, radius, 0);
            corners[5].set(innerL, outerT, innerTL, outerTT, 0, radius);

            // Bottom right
            corners[6].set(outerR, outerB, outerTR, outerTB, radius, radius);
            corners[7].set(outerR, innerB, outerTR, innerTB, radius, 0);
            corners[8].set(innerR, outerB, innerTR, outerTB, 0, radius);

            // Top right
            corners[9].set(outerR, outerT, outerTR, outerTT, radius, radius);
            corners[10].set(outerR, innerT, outerTR, innerTT, radius, 0);
            corners[11].set(innerR, outerT, innerTR, outerTT, 0, radius);

            m_cornerNode.markDirty(DirtyGeometry);
        } else {
            m_geometry.allocate(4);
            QSGGeometry::updateTexturedRectGeometry(&m_geometry, m_rect, textureRect);
        }

        markDirty(DirtyGeometry);
    }
}

void SurfaceNode::setBlending(bool b)
{
    m_opaqueMaterial.setFlag(QSGMaterial::Blending, b);
}

void SurfaceNode::setRadius(qreal radius)
{
    if (m_radius == radius)
        return;

    if (m_radius == 0 && radius != 0) {
        appendChildNode(&m_cornerNode);
    } else if (m_radius != 0 && radius == 0) {
        removeChildNode(&m_cornerNode);
    }

    m_radius = radius;
    m_geometryChanged = true;
    m_cornerMaterial.setRadius(m_radius);

    m_cornerNode.markDirty(DirtyMaterial);
}

void SurfaceNode::setTexture(QSGTexture *texture)
{
    m_opaqueMaterial.setTexture(texture);

    QRectF tr;
    if (texture) tr = texture->convertToNormalizedSourceRect(QRect(QPoint(0,0), texture->textureSize()));

    m_geometryChanged |= !m_texture || tr != m_textureRect;

    m_texture = texture;
    m_textureRect = tr;

    m_texture = texture;

    markDirty(DirtyMaterial);
    if (m_radius > 0) {
        m_cornerNode.markDirty(DirtyMaterial);
    }
}

void SurfaceNode::setXOffset(qreal offset)
{
    m_geometryChanged |= m_xOffset != offset;

    m_xOffset = offset;
}

void SurfaceNode::setYOffset(qreal offset)
{
    m_geometryChanged |= m_yOffset != offset;

    m_yOffset = offset;
}

void SurfaceNode::setXScale(qreal xScale)
{
    m_geometryChanged |= m_xScale != xScale;

    m_xScale = xScale;
}

void SurfaceNode::setYScale(qreal yScale)
{
    m_geometryChanged |= m_yScale != yScale;

    m_yScale = yScale;
}

void SurfaceNode::textureChanged()
{
    setTexture(m_provider->texture());
    updateGeometry();
}

void SurfaceNode::providerDestroyed()
{
    m_provider = 0;
    setTexture(0);
}
