#pragma once
#ifndef USE_VULKAN

#include <GL/glew.h>

#include <string>
#include <vector>

#include "ResourceBase.h"

// DDS forward declaration
namespace DirectX
{
    struct DDS_HEADER;
}

using namespace DirectX;

enum class TextureType
{
    Diffuse = 0,
    Normal,
    Specular,
    Skybox
};

class Texture : public ResourceBase
{
private:

    TextureType m_TextureType;

    GLuint m_TextureHandle;
    GLuint m_Width;
    GLuint m_Height;
    GLuint m_MipmapLevel;

public:

    Texture(const std::string& resourceName = "", const std::filesystem::path& filePath = "");
    ~Texture();

    // Inherited via FileResourceBase
    virtual void Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentElem) override;
    virtual void Unserialize(tinyxml2::XMLElement* currElem) override;
    virtual void Load() override;

    void LoadDDS();
    void LoadTextureDDS(DirectX::DDS_HEADER const& header, std::ifstream& fs);
    void LoadSkyboxDDS(DirectX::DDS_HEADER const& header, std::ifstream& fs);

    void ImportTexture(const std::filesystem::path& filePath);
    void ImportSkybox(std::vector<std::filesystem::path> const& faces);

    GLuint GetHandle() const;
};

#endif  // !USE_VULKAN