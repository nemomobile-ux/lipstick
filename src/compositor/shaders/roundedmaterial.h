#ifndef ROUNDEDMATERIAL_H
#define ROUNDEDMATERIAL_H

#include <QSGMaterial>

class RoundedMaterial : public QSGMaterial
{
public:
    RoundedMaterial();

    QSGMaterialType *type() const override;
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode) const override;
    int compare(const QSGMaterial *other) const override;

    void setTexture(QSGTexture *texture);

    QSGTexture *texture() const;

    void setRadius(float radius);

    float radius() const;

    void setSize(const QSizeF &size);

    QSizeF size() const;

private:
    QSGTexture *m_texture = nullptr;
    float m_radius = 0.f;
    QSizeF m_size;
};

#endif // ROUNDEDMATERIAL_H
