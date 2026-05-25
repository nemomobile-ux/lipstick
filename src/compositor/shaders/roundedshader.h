#ifndef ROUNDEDSHADER_H
#define ROUNDEDSHADER_H

#include <QSGMaterialShader>

struct UniformData
{
    QMatrix4x4 matrix;
    float opacity;
    float radius;
    QVector2D size;
};

class RoundedShader : public QSGMaterialShader
{
public:
    RoundedShader();

    bool updateUniformData(
        RenderState &state,
        QSGMaterial *newMaterial,
        QSGMaterial *) override;

    void updateSampledImage(
        RenderState &,
        int binding,
        QSGTexture **texture,
        QSGMaterial *newMaterial,
        QSGMaterial *) override;
};

#endif // ROUNDEDSHADER_H
