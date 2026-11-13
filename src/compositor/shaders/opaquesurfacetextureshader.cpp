#include "opaquesurfacetextureshader.h"
#include "opaquesurfacetexturematerial.h"

OpaqueSurfaceTextureShader::OpaqueSurfaceTextureShader()
{
    setShaderFileName(
        VertexStage,
        QStringLiteral(":/compositor/shaders/opaque.vert.qsb"));

    setShaderFileName(
        FragmentStage,
        QStringLiteral(":/compositor/shaders/opaque.frag.qsb"));
}

bool OpaqueSurfaceTextureShader::updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    bool changed = false;
    QByteArray *buf = state.uniformData();
    if (!buf)
        return changed;

    constexpr int matrixSize = 64;

    if (buf->size() < matrixSize)
        buf->resize(matrixSize);

    if (state.isMatrixDirty()) {
        const QMatrix4x4 m = state.combinedMatrix();
        memcpy(buf->data(), m.constData(), 64);
        changed = true;
    }

    return changed;
}

void OpaqueSurfaceTextureShader::updateSampledImage(
    RenderState &state,
    int binding,
    QSGTexture **texture,
    QSGMaterial *newMaterial,
    QSGMaterial *oldMaterial)
{
    Q_UNUSED(state)
    Q_UNUSED(oldMaterial)

    if (binding != 1)
        return;

    auto *material =
        static_cast<OpaqueSurfaceTextureMaterial *>(newMaterial);

    *texture = material->texture();

    Q_ASSERT(*texture);
}
