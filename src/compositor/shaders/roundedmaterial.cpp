#include "roundedmaterial.h"
#include "roundedshader.h"

RoundedMaterial::RoundedMaterial()
{
    setFlag(Blending, true);
}

QSGMaterialType *RoundedMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *RoundedMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new RoundedShader;
}

int RoundedMaterial::compare(const QSGMaterial *other) const
{
    auto *o =
        static_cast<const RoundedMaterial *>(other);

    if (m_texture == o->m_texture)
        return 0;

    return m_texture < o->m_texture ? -1 : 1;
}

void RoundedMaterial::setTexture(QSGTexture *texture)
{
    m_texture = texture;
}

QSGTexture *RoundedMaterial::texture() const
{
    return m_texture;
}

void RoundedMaterial::setRadius(float radius)
{
    m_radius = radius;
}

float RoundedMaterial::radius() const
{
    return m_radius;
}

void RoundedMaterial::setSize(const QSizeF &size)
{
    m_size = size;
}

QSizeF RoundedMaterial::size() const
{
    return m_size;
}
