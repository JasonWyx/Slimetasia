#ifdef USE_VULKAN
#include "Texture_Vulkan.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>

#include "DDS.h"

Texture::Texture(const std::string& resourceName, const std::filesystem::path& filePath)
    : ResourceBase(resourceName, filePath)
{
}

Texture::~Texture() {}

void Texture::Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentElem)
{
    tinyxml2::XMLElement* resourceElem = doc->NewElement("Texture");
    tinyxml2::XMLElement* nameElem = doc->NewElement("Name");
    tinyxml2::XMLElement* filepathElem = doc->NewElement("Filepath");

    resourceElem->SetAttribute("GUID", static_cast<int64_t>(m_GUID));
    nameElem->SetText(m_Name.c_str());
    filepathElem->SetText(m_FilePath.string().c_str());

    parentElem->InsertEndChild(resourceElem);
    resourceElem->InsertEndChild(nameElem);
    resourceElem->InsertEndChild(filepathElem);
}

void Texture::Unserialize(tinyxml2::XMLElement* currElem)
{
    m_GUID = (ResourceGUID)currElem->Int64Attribute("GUID");

    m_Name = currElem->FirstChildElement("Name")->GetText();
    m_FilePath = currElem->FirstChildElement("Filepath")->GetText();
}

void Texture::Load()
{
    int width, height, channels;
    if (m_FilePath.extension() == ".dds")
    {
        LoadDDS();
    }
    else
    {
        stbi_set_flip_vertically_on_load(1);

        unsigned char* imageData = stbi_load(m_FilePath.string().c_str(), &width, &height, &channels, 0);
        // todo: Load normal texture
        stbi_image_free(imageData);
    }

    m_LoadStatus = ResourceStatus::Loaded;
}

void Texture::ImportTexture(const std::filesystem::path& filePath)
{
    stbi_set_flip_vertically_on_load(1);

    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* imageData = stbi_load(filePath.string().c_str(), &width, &height, &channels, 0);

    vk::Format format = FromChannelCountToVkFormat(channels);
    // todo: Convert to DDS format and write to file
    unsigned imageSize = 2;

    char* imagePixels = new char[imageSize];

    // Setup DDS file header
    DirectX::DDS_HEADER header;
    header.size = sizeof(header);
    header.width = width;
    header.height = height;
    header.ddspf = FromVkFormatToDDSPixelFormat(format);
    header.mipMapCount = 1;
    header.pitchOrLinearSize = imageSize;
    header.caps = DDS_SURFACE_FLAGS_TEXTURE;
    header.flags = DDS_HEADER_FLAGS_TEXTURE;

    m_FilePath.replace_extension(".dds");

    std::ofstream ofs = std::ofstream(m_FilePath, std::ios::binary);

    ofs.write((char*)&DirectX::DDS_MAGIC, sizeof(DirectX::DDS_MAGIC));
    ofs.write((char*)&header, sizeof(header));
    ofs.write(imagePixels, imageSize);

    ofs.close();

    delete[] imagePixels;

    stbi_image_free(imageData);

    m_LoadStatus = ResourceStatus::Loaded;
}

void Texture::ImportSkybox(const std::vector<std::filesystem::path>& faces)
{
    stbi_set_flip_vertically_on_load(0);

    int width = 0;
    int height = 0;
    int channels = 0;

    // Load first cubemap face; positive x
    unsigned char* imageData = stbi_load(faces[0].string().c_str(), &width, &height, &channels, 0);
    unsigned imageSize = 10;
    vk::Format format = FromChannelCountToVkFormat(channels);

    // todo: Load first face into X+ plane

    char* imagePixels = new char[imageSize];

    DirectX::DDS_HEADER header;
    header.size = sizeof(header);
    header.width = width;
    header.height = height;
    header.ddspf = FromVkFormatToDDSPixelFormat(format);
    header.mipMapCount = 1;
    header.pitchOrLinearSize = imageSize;
    header.caps = DDS_SURFACE_FLAGS_CUBEMAP;
    header.flags = DDS_HEADER_FLAGS_TEXTURE;

    // todo: Load cubemap faces

    stbi_image_free(imageData);

    m_FilePath.replace_extension(".dds");

    std::ofstream ofs = std::ofstream(m_FilePath, std::ios::binary);

    ofs.write((char*)&DirectX::DDS_MAGIC, sizeof(DirectX::DDS_MAGIC));
    ofs.write((char*)&header, sizeof(header));
    ofs.write(imagePixels, imageSize);

    // Load other cubemap faces
    for (std::size_t i = 1; i < faces.size(); ++i)
    {
        imageData = stbi_load(faces[i].string().c_str(), &width, &height, &channels, 0);

        // todo: Load each cubemap into image object

        stbi_image_free(imageData);

        ofs.write(imagePixels, imageSize);
    }

    ofs.close();
    delete[] imagePixels;
    m_LoadStatus = ResourceStatus::Loaded;
}

void Texture::LoadDDS()
{
    // Verify tag and dds header
    uint32_t tag;
    DirectX::DDS_HEADER header;

    std::ifstream fs = std::ifstream(m_FilePath, std::ios::binary);

    fs.read((char*)&tag, sizeof(tag));

    if (!fs.is_open())
    {
        printf_s("ERROR: Failed to load texture - %s could not be opened.\n", m_FilePath.string().c_str());
        return;
    }

    if (tag != DirectX::DDS_MAGIC)
    {
        printf_s("ERROR: Failed to load texture - %s is not a valid DDS file.\n", m_FilePath.string().c_str());
        fs.close();
        return;
    }

    fs.read((char*)&header, sizeof(header));

    if (header.caps & DDS_SURFACE_FLAGS_CUBEMAP)
    {
        LoadSkyboxDDS(header, fs);
    }
    else
    {
        LoadTextureDDS(header, fs);
    }
}

void Texture::LoadTextureDDS(DirectX::DDS_HEADER const& header, std::ifstream& fs)
{
    const unsigned linearSize = header.pitchOrLinearSize;
    const unsigned mipMapCount = header.mipMapCount;
    const unsigned width = header.width;
    const unsigned height = header.height;
    const unsigned fourCC = header.ddspf.fourCC;
    const vk::Format format = FromFourCCToVkFormat(fourCC);
    const vk::DeviceSize blockSize = format == vk::Format::eBc1RgbaUnormBlock ? 8 : 16;

    const unsigned bufferSize = mipMapCount > 1 ? linearSize * 2 : linearSize;
    unsigned char* buffer = new unsigned char[bufferSize];

    fs.read((char*)buffer, bufferSize);

    // todo: Load DDS texture into image object

    unsigned bufferOffset = 0;
    unsigned currWidth = width;
    unsigned currHeight = height;

    for (unsigned mipLevel = 0; mipLevel < 1; ++mipLevel)
    {
        unsigned imageSize = ((currWidth + 3) / 4) * ((currHeight + 3) / 4) * blockSize;

        // todo: Load each mipmap (might need to extend ImageObject)

        bufferOffset += imageSize;
        currWidth /= 2;
        currHeight /= 2;
    }

    delete[] buffer;
}

void Texture::LoadSkyboxDDS(DirectX::DDS_HEADER const& header, std::ifstream& fs)
{
    const unsigned linearSize = header.pitchOrLinearSize;
    const unsigned mipCount = header.mipMapCount;
    const unsigned width = header.width;
    const unsigned height = header.height;
    const unsigned fourCC = header.ddspf.fourCC;
    const vk::Format format = FromFourCCToVkFormat(fourCC);
    const unsigned blockSize = format == vk::Format::eBc1RgbaUnormBlock ? 8 : 16;

    unsigned bufferSize = mipCount > 1 ? linearSize * 2 : linearSize;
    unsigned char* buffer = new unsigned char[bufferSize];

    fs.read((char*)buffer, bufferSize);

    for (unsigned i = 0; i < 6; ++i)
    {
        unsigned offset = 0;
        unsigned currWidth = width;
        unsigned currHeight = height;

        for (unsigned mipLevel = 0; mipLevel < mipCount; ++mipLevel)
        {
            unsigned imageSize = ((currWidth + 3) / 4) * ((currHeight + 3) / 4) * blockSize;

            // todo: Load mips for each cubemap
            // glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mipLevel, format, currWidth, currHeight, 0, imageSize, buffer + offset);

            offset += imageSize;
            currWidth /= 2;
            currHeight /= 2;
        }
    }
    delete[] buffer;
}

#endif  // !USE_VULKAN

vk::Format Texture::FromChannelCountToVkFormat(uint32_t channelCount)
{
    switch (channelCount)
    {
        case 3: return vk::Format::eBc1RgbUnormBlock;
        case 4: return vk::Format::eBc5UnormBlock;
    }
    return vk::Format::eUndefined;
}

vk::Format Texture::FromFourCCToVkFormat(uint32_t fourCC)
{
    if (fourCC == DirectX::DDSPF_DXT1.fourCC)
    {
        return vk::Format::eBc1RgbaUnormBlock;
    }
    else if (fourCC == DirectX::DDSPF_DXT3.fourCC)
    {
        return vk::Format::eBc3UnormBlock;
    }
    else if (fourCC == DirectX::DDSPF_DXT5.fourCC)
    {
        return vk::Format::eBc5UnormBlock;
    }
    return vk::Format::eUndefined;
}

DirectX::DDS_PIXELFORMAT Texture::FromVkFormatToDDSPixelFormat(vk::Format format)
{
    if (format == vk::Format::eBc1RgbaUnormBlock)
    {
        return DirectX::DDSPF_DXT1;
    }
    else if (format == vk::Format::eBc3UnormBlock)
    {
        return DirectX::DDSPF_DXT3;
    }
    else if (format == vk::Format::eBc5UnormBlock)
    {
        return DirectX::DDSPF_DXT5;
    }
    return DirectX::DDS_PIXELFORMAT {};
}
