#ifndef ROUNDEDCLIPMATERIAL_H
#define ROUNDEDCLIPMATERIAL_H

#include <QSGMaterial>
#include <QSGTexture>

class RoundedClipMaterial : public QSGMaterial
{
public:
    RoundedClipMaterial();

    void setTexture(QSGTexture *texture) { m_texture = texture; }
    QSGTexture *texture() const { return m_texture; }

    void setRadius(float r) { m_radius = r; }
    float radius() const { return m_radius; }

    void setItemSize(const QSizeF &s) { m_itemSize = s; }
    QSizeF itemSize() const { return m_itemSize; }

    QSGMaterialType *type() const override;
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode renderMode) const override;
    int compare(const QSGMaterial *other) const override;

private:
    QSGTexture *m_texture = nullptr;
    float m_radius = 0.0f;
    QSizeF m_itemSize;
};

#endif
