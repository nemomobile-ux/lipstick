#ifndef SURFACETEXTUREMATERIAL_H
#define SURFACETEXTUREMATERIAL_H

#include <QSGMaterial>
#include "opaquesurfacetexturematerial.h"


class SurfaceTextureMaterial : public QSGMaterial
{
public:
    SurfaceTextureMaterial(OpaqueSurfaceTextureMaterial &opaqueMaterial);
    QSGTexture *texture() const;
    void setTexture(QSGTexture *texture);

    QSGMaterialType *type() const override;
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode renderMode) const override;

    int compare(const QSGMaterial *other) const override;

    OpaqueSurfaceTextureMaterial &opaqueMaterial;

protected:
    QSGTexture *m_texture = nullptr;
};

#endif // SURFACETEXTUREMATERIAL_H
