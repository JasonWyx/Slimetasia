#include "Material.h"

void Material::Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* root)
{
    ResourceBase::Serialize(doc, root);
}

void Material::Unserialize(tinyxml2::XMLElement* root)
{
    ResourceBase::Unserialize(root);
}

HTexture Material::GetAlbedoTexture() const
{
    return m_DiffuseTexture;
}

void Material::SetDiffuseTexture(HTexture texture)
{
    m_DiffuseTexture = texture;
}

HTexture Material::GetSpecularTexture() const
{
    return m_SpecularTexture;
}

void Material::SetSpecularTexture(HTexture texture)
{
    m_SpecularTexture = texture;
}

HTexture Material::GetNormalTexture() const
{
    return m_NormalTexture;
}

void Material::SetNormalTexture(HTexture texture)
{
    m_NormalTexture = texture;
}
