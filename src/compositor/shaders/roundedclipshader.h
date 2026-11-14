#ifndef ROUNDEDCLIPSHADER_H
#define ROUNDEDCLIPSHADER_H

#include <QSGMaterialShader>

class RoundedClipShader : public QSGMaterialShader
{
public:
    RoundedClipShader();

    bool updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
    void updateSampledImage(RenderState &state, int binding, QSGTexture **texture,
                            QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
};

#endif
