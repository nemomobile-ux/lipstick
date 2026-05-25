#include "opaquesurfacetexturematerial.h"
#include "opaquesurfacetextureshader.h"
#include <qsgtexture.h>

OpaqueSurfaceTextureMaterial::OpaqueSurfaceTextureMaterial()
{
}

QSGMaterialType *OpaqueSurfaceTextureMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *OpaqueSurfaceTextureMaterial::createShader(QSGRendererInterface::RenderMode renderMode) const
{
    return new OpaqueSurfaceTextureShader;
}

int OpaqueSurfaceTextureMaterial::compare(const QSGMaterial *other) const
{
    const OpaqueSurfaceTextureMaterial * const surface = static_cast<const OpaqueSurfaceTextureMaterial *>(other);

    return (m_texture ? m_texture->comparisonKey() : 0)
           - (surface->m_texture ? surface->m_texture->comparisonKey() : 0);
}
