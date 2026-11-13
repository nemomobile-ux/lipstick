#include "surfacetexturematerial.h"
#include "opaquesurfacetextureshader.h"

SurfaceTextureMaterial::SurfaceTextureMaterial(OpaqueSurfaceTextureMaterial &opaqueMaterial)
    : opaqueMaterial(opaqueMaterial)
{
}

QSGTexture *SurfaceTextureMaterial::texture() const
{
    return m_texture;
}

void SurfaceTextureMaterial::setTexture(QSGTexture *texture)
{
    m_texture = texture;
}

QSGMaterialType *SurfaceTextureMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *SurfaceTextureMaterial::createShader(QSGRendererInterface::RenderMode renderMode) const
{
    return new OpaqueSurfaceTextureShader;
}

int SurfaceTextureMaterial::compare(const QSGMaterial *other) const
{
    return opaqueMaterial.compare(&static_cast<const SurfaceTextureMaterial *>(other)->opaqueMaterial);
}
