#ifndef OPAQUESURFACETEXTUREMATERIAL_H
#define OPAQUESURFACETEXTUREMATERIAL_H

#include <QSGMaterial>


class OpaqueSurfaceTextureMaterial : public QSGMaterial
{
public:
    OpaqueSurfaceTextureMaterial();
    void setTexture(QSGTexture *texture) { m_texture = texture; }
    QSGTexture *texture() const { return m_texture; }

    QSGMaterialType *type() const override;
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode renderMode) const override;

    int compare(const QSGMaterial *other) const override;

private:
    QSGTexture *m_texture = nullptr;
};

#endif // OPAQUESURFACETEXTUREMATERIAL_H
