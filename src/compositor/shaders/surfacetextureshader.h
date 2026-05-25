#ifndef SURFACETEXTURESHADER_H
#define SURFACETEXTURESHADER_H

#include "opaquesurfacetextureshader.h"

class SurfaceTextureShader : public OpaqueSurfaceTextureShader
{
public:
    SurfaceTextureShader();
    bool updateUniformData(RenderState &state
                           , QSGMaterial *newMaterial
                           , QSGMaterial *oldMaterial) override;

private:
    int m_id_qt_Opacity = -1;
};

#endif // SURFACETEXTURESHADER_H
