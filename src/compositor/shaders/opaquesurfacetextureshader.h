#ifndef OPAQUESURFACETEXTURESHADER_H
#define OPAQUESURFACETEXTURESHADER_H

#include <QSGMaterialShader>

class OpaqueSurfaceTextureShader : public QSGMaterialShader
{
public:
    OpaqueSurfaceTextureShader();
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
    int m_id_qt_Matrix = -1;
    int m_qt_Texture = -1;
};

#endif // OPAQUESURFACETEXTURESHADER_H
