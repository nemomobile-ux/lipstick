#include "roundedshader.h"
#include "roundedmaterial.h"

RoundedShader::RoundedShader()
{
    setShaderFileName(
        VertexStage,
        QStringLiteral(":/compositor/shaders/rounded.vert.qsb"));

    setShaderFileName(
        FragmentStage,
        QStringLiteral(":/compositor/shaders/rounded.frag.qsb"));
}

bool RoundedShader::updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *)
{
    QByteArray *buf = state.uniformData();

    if (buf->size() < int(sizeof(UniformData))) {
        buf->resize(sizeof(UniformData));
    }

    UniformData *data = reinterpret_cast<UniformData *>(buf->data());

    if (state.isMatrixDirty()) {
        data->matrix = state.combinedMatrix();
    }

    if (state.isOpacityDirty()) {
        data->opacity = state.opacity();
    }

    RoundedMaterial *material = static_cast<RoundedMaterial *>(newMaterial);

    data->radius = material->radius();

    data->size = QVector2D(material->size().width(), material->size().height());

    return true;
}

void RoundedShader::updateSampledImage(RenderState &, int binding, QSGTexture **texture, QSGMaterial *newMaterial, QSGMaterial *)
{
    if (binding != 1)
        return;

    RoundedMaterial *material = static_cast<RoundedMaterial *>(newMaterial);

    *texture = material->texture();
}
