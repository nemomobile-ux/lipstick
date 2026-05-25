#include "cornersurfacetextureshader.h"
#include "cornersurfacetexturematerial.h"

CornerSurfaceTextureShader::CornerSurfaceTextureShader()
{
    setShaderFileName(
        VertexStage,
        QStringLiteral(":/compositor/shaders/corner.vert.qsb"));

    setShaderFileName(
        FragmentStage,
        QStringLiteral(":/compositor/shaders/corner.frag.qsb"));
}

bool CornerSurfaceTextureShader::updateUniformData(
    RenderState &state,
    QSGMaterial *newMaterial,
    QSGMaterial *oldMaterial)
{
    bool changed =
        SurfaceTextureShader::updateUniformData(
            state,
            newMaterial,
            oldMaterial);

    auto *newSurface =
        static_cast<CornerSurfaceTextureMaterial *>(newMaterial);

    auto *oldSurface =
        static_cast<CornerSurfaceTextureMaterial *>(oldMaterial);

    QByteArray *buf = state.uniformData();

    if (!buf)
        return changed;

    constexpr int UBUF_SIZE = 96;

    if (buf->size() < UBUF_SIZE)
        buf->resize(UBUF_SIZE);

    const float radius = newSurface->radius();

    const float val1 =
        (radius + 0.5f) * (radius + 0.5f);

    const float val2 =
        (radius - 0.5f) * (radius - 0.5f);

    if (!oldSurface || oldSurface->radius() != radius) {

        float radiusData[2] = {
            val1,
            val2
        };

        // std140 vec2 radius offset = 80
        memcpy(buf->data() + 80,
               radiusData,
               sizeof(radiusData));

        changed = true;
    }

    return changed;
}

void CornerSurfaceTextureShader::updateSampledImage(
    RenderState &state,
    int binding,
    QSGTexture **texture,
    QSGMaterial *newMaterial,
    QSGMaterial *oldMaterial)
{
    Q_UNUSED(state)
    Q_UNUSED(oldMaterial)

    auto *material =
        static_cast<CornerSurfaceTextureMaterial *>(newMaterial);

    if (binding != 1)
        return;

    *texture = material->texture();
}
