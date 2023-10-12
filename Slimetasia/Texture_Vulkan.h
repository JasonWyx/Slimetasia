#pragma once
#ifdef USE_VULKAN
#include "ImageObject.h"
#include "ResourceBase.h"

namespace DirectX
{
    struct DDS_HEADER;
    struct DDS_PIXELFORMAT;
}
 
class Texture : public ResourceBase
{
public:

    Texture(const std::string& resourceName = "", const std::filesystem::path& filePath = "");
    ~Texture();

    ImageObject* GetImageObject() { return m_ImageObject; }
    const ImageObject* GetImageObject() const { return m_ImageObject; }

    // Inherited via FileResourceBase
    void Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentElem) override;
    void Unserialize(tinyxml2::XMLElement* currElem) override;
    void Load() override;

    void ImportTexture(const std::filesystem::path& filePath);
    void ImportSkybox(std::vector<std::filesystem::path> const& faces);

private:

    void LoadDDS();
    void LoadTextureDDS(DirectX::DDS_HEADER const& header, std::ifstream& fs);
    void LoadSkyboxDDS(DirectX::DDS_HEADER const& header, std::ifstream& fs);

    static vk::Format FromChannelCountToVkFormat(uint32_t channelCount);
    static vk::Format FromFourCCToVkFormat(uint32_t fourCC);
    static DirectX::DDS_PIXELFORMAT FromVkFormatToDDSPixelFormat(vk::Format format);

    ImageObject* m_ImageObject = nullptr;
};

#endif  // USE_VULKAN