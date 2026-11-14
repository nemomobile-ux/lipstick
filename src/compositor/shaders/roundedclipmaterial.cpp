#include "roundedclipmaterial.h"
#include "roundedclipshader.h"

RoundedClipMaterial::RoundedClipMaterial()
{
    setFlag(Blending);
}

QSGMaterialType *RoundedClipMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *RoundedClipMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new RoundedClipShader();
}

int RoundedClipMaterial::compare(const QSGMaterial *other) const
{
    const auto *o = static_cast<const RoundedClipMaterial *>(other);
    if (m_texture != o->m_texture)
        return m_texture < o->m_texture ? -1 : 1;
    if (m_radius != o->m_radius)
        return m_radius < o->m_radius ? -1 : 1;
    if (m_itemSize != o->m_itemSize)
        return m_itemSize.width() < o->m_itemSize.width() ? -1 : 1;
    return 0;
}
