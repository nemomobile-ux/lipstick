#ifndef CORNERSURFACETEXTUREMATERIAL_H
#define CORNERSURFACETEXTUREMATERIAL_H

#include "surfacetexturematerial.h"

class CornerSurfaceTextureMaterial : public SurfaceTextureMaterial
{
public:
    CornerSurfaceTextureMaterial(OpaqueSurfaceTextureMaterial &opaqueMaterial);

    float radius() const { return m_radius; }
    void setRadius(float radius) { m_radius = radius; }

    QSGMaterialType *type() const override;
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode renderMode) const override;

    int compare(const QSGMaterial *other) const override;

private:
    qreal m_radius = 0;
};

#endif // CORNERSURFACETEXTUREMATERIAL_H
