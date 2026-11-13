#ifndef SURFACENODE_H
#define SURFACENODE_H

#include <QSGGeometryNode>
#include <QSGTexture>
#include <QSGTextureProvider>

#include "cornersurfacetexturematerial.h"

struct CornerVertex
{
    float m_x;
    float m_y;
    float m_tx;
    float m_ty;
    float m_cx;
    float m_cy;

    void set(float x, float y, float tx, float ty, float cx, float cy)
    {
        m_x = x; m_y = y; m_tx = tx; m_ty = ty; m_cx = cx; m_cy = cy;
    }
};

class SurfaceNode : public QObject, public QSGGeometryNode
{
    Q_OBJECT
public:
    SurfaceNode();
    ~SurfaceNode();
    void setRect(const QRectF &);
    void setTextureProvider(QSGTextureProvider *p, bool owned);
    void setBlending(bool);
    void setRadius(qreal radius);
    void setXOffset(qreal xOffset);
    void setYOffset(qreal yOffset);
    void setXScale(qreal xScale);
    void setYScale(qreal yScale);

    void updateGeometry();

    const QSGGeometry::AttributeSet &cornerAttributes()
    {
        static QSGGeometry::Attribute data[] = {
            QSGGeometry::Attribute::create(0, 2, GL_FLOAT, true),
            QSGGeometry::Attribute::create(1, 2, GL_FLOAT),
            QSGGeometry::Attribute::create(2, 2, GL_FLOAT)
        };
        static QSGGeometry::AttributeSet attributes = { 3, sizeof(CornerVertex), data };
        return attributes;
    }

private slots:
    void providerDestroyed();
    void textureChanged();

private:
    void setTexture(QSGTexture *texture);
    OpaqueSurfaceTextureMaterial m_opaqueMaterial;
    SurfaceTextureMaterial m_material { m_opaqueMaterial };
    CornerSurfaceTextureMaterial m_cornerMaterial { m_opaqueMaterial };
    QSGGeometry m_geometry { QSGGeometry::defaultAttributes_TexturedPoint2D(), 0 };
    QSGGeometry m_cornerGeometry { cornerAttributes(), 0 };
    QSGGeometryNode m_cornerNode;

    QRectF m_rect;
    QRectF m_textureRect;
    qreal m_radius = 0;
    qreal m_xOffset = 0;
    qreal m_yOffset = 0;
    qreal m_xScale = 1;
    qreal m_yScale = 1;

    QPointer<QSGTextureProvider> m_provider;
    QSGTexture *m_texture = nullptr;
    bool m_providerOwned = false;
    bool m_geometryChanged = true;
};

#endif // SURFACENODE_H
