#include "surfacetextureshader.h"
#include "surfacetexturematerial.h"

SurfaceTextureShader::SurfaceTextureShader() {
    setShaderFileName(
        VertexStage,
        QStringLiteral(":/compositor/shaders/surface.vert.qsb"));

    setShaderFileName(
        FragmentStage,
        QStringLiteral(":/compositor/shaders/surface.frag.qsb"));
}

bool SurfaceTextureShader::updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    SurfaceTextureMaterial *newSurface = static_cast<SurfaceTextureMaterial*>(newMaterial);
    SurfaceTextureMaterial *oldSurface = static_cast<SurfaceTextureMaterial*>(oldMaterial);

    bool changed = false;
    if (newSurface) {
        changed |= OpaqueSurfaceTextureShader::updateUniformData(
            state,
            &newSurface->opaqueMaterial,
            oldSurface ? &oldSurface->opaqueMaterial : nullptr
            );
    } else if (oldSurface) {
        changed |= OpaqueSurfaceTextureShader::updateUniformData(
            state,
            oldSurface ? &oldSurface->opaqueMaterial : nullptr,
            &oldSurface->opaqueMaterial
            );
    }

    if (state.isOpacityDirty()) {
        QByteArray *buf = state.uniformData();
        if (buf) {
            float opacity = state.opacity();
            memcpy(buf->data(), &opacity, sizeof(float));
            changed = true;
        }
    }

    return changed;
}
