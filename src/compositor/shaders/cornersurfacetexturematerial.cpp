#include "cornersurfacetexturematerial.h"
#include "cornersurfacetextureshader.h"

CornerSurfaceTextureMaterial::CornerSurfaceTextureMaterial(OpaqueSurfaceTextureMaterial &opaqueMaterial)
    : SurfaceTextureMaterial(opaqueMaterial)
{
}

QSGMaterialType *CornerSurfaceTextureMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *CornerSurfaceTextureMaterial::createShader(QSGRendererInterface::RenderMode renderMode) const
{
    return new CornerSurfaceTextureShader;
}

int CornerSurfaceTextureMaterial::compare(const QSGMaterial *other) const
{
    const CornerSurfaceTextureMaterial * const surface = static_cast<const CornerSurfaceTextureMaterial *>(other);

    const int result = SurfaceTextureMaterial::compare(other);
    if (result != 0) {
        return result;
    } else if (m_radius < surface->m_radius) {
        return -1;
    } else if (m_radius > surface->m_radius) {
        return 1;
    } else {
        return 0;
    }
}
