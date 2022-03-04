#pragma once
#include "ResourceBase.h"
#include "ResourceHandle.h"
#include "Texture.h"

using HTexture = ResourceHandle<Texture>;

class Material : public ResourceBase
{
    enum DispMapType
    {
        BumpMap,
        NormalMap
    };

    HTexture m_DiffuseTexture;
    HTexture m_SpecularTexture;
    HTexture m_NormalTexture;

public:
    Material() = default;
    ~Material() = default;

    // Inherited via ResourceBase
    virtual void Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* root) override;
    virtual void Unserialize(tinyxml2::XMLElement* root) override;

    HTexture GetAlbedoTexture() const;
    void SetDiffuseTexture(HTexture texture);

    HTexture GetSpecularTexture() const;
    void SetSpecularTexture(HTexture texture);

    HTexture GetNormalTexture() const;
    void SetNormalTexture(HTexture texture);
};
