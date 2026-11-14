#include "roundedclipshader.h"
#include "roundedclipmaterial.h"

#include <cstring>

RoundedClipShader::RoundedClipShader()
{
    setShaderFileName(VertexStage, QStringLiteral(":/compositor/shaders/roundedclip.vert.qsb"));
    setShaderFileName(FragmentStage, QStringLiteral(":/compositor/shaders/roundedclip.frag.qsb"));
}

bool RoundedClipShader::updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    auto *mat = static_cast<RoundedClipMaterial *>(newMaterial);
    auto *oldMat = static_cast<RoundedClipMaterial *>(oldMaterial);

    bool changed = false;
    QByteArray *buf = state.uniformData();
    if (buf->size() < 80)
        buf->resize(80);

    if (state.isMatrixDirty()) {
        const QMatrix4x4 m = state.combinedMatrix();
        memcpy(buf->data(), m.constData(), 64);
        changed = true;
    }

    if (state.isOpacityDirty()) {
        const float opacity = state.opacity();
        memcpy(buf->data() + 64, &opacity, sizeof(float));
        changed = true;
    }

    const float radius = mat ? mat->radius() : 0.0f;
    if (!oldMat || oldMat->radius() != radius) {
        memcpy(buf->data() + 68, &radius, sizeof(float));
        changed = true;
    }

    const QSizeF size = mat ? mat->itemSize() : QSizeF();
    if (!oldMat || oldMat->itemSize() != size) {
        float s[2] = { float(size.width()), float(size.height()) };
        memcpy(buf->data() + 72, s, sizeof(s));
        changed = true;
    }

    return changed;
}

void RoundedClipShader::updateSampledImage(RenderState &, int binding, QSGTexture **texture,
                                           QSGMaterial *newMaterial, QSGMaterial *)
{
    if (binding != 1)
        return;
    auto *mat = static_cast<RoundedClipMaterial *>(newMaterial);
    *texture = mat ? mat->texture() : nullptr;
}
