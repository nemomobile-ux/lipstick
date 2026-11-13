#ifndef CORNERSURFACETEXTURESHADER_H
#define CORNERSURFACETEXTURESHADER_H

#include "surfacetextureshader.h"

class CornerSurfaceTextureShader : public SurfaceTextureShader
{
public:
    CornerSurfaceTextureShader();
    bool updateUniformData(RenderState &state
                           , QSGMaterial *newMaterial
                           , QSGMaterial *oldMaterial) override;

    void updateSampledImage(
        RenderState &state,
        int binding,
        QSGTexture **texture,
        QSGMaterial *newMaterial,
        QSGMaterial *oldMaterial) override;

private:
    int m_id_radius;
};

#endif // CORNERSURFACETEXTURESHADER_H
