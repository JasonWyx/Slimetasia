#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>

#include "DDS.h"

Texture::Texture(const std::string& resourceName, const std::filesystem::path& filePath)
    : ResourceBase(resourceName, filePath)
    , m_TextureType(TextureType::Diffuse)
    , m_TextureHandle(0)
    , m_Width(0)
    , m_Height(0)
    , m_MipmapLevel(0)
{
    glGenTextures(1, &m_TextureHandle);
}

Texture::~Texture()
{
    if (m_TextureHandle != 0) glDeleteTextures(1, &m_TextureHandle);
}

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

        glBindTexture(GL_TEXTURE_2D, m_TextureHandle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, channels == 3 ? GL_RGB : GL_RGBA, width, height, 0, channels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, imageData);
        stbi_image_free(imageData);
    }

    m_LoadStatus = ResourceStatus::Loaded;
}

void Texture::LoadDDS()
{
    // Verify tag and dds header
    uint32_t tag;
    DDS_HEADER header;

    std::ifstream fs = std::ifstream(m_FilePath, std::ios::binary);

    fs.read((char*)&tag, sizeof(tag));

    if (!fs.is_open())
    {
        printf_s("ERROR: Failed to load texture - %s could not be opened.\n", m_FilePath.string().c_str());
        return;
    }

    if (tag != DDS_MAGIC)
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
    const GLuint format = fourCC == DDSPF_DXT1.fourCC   ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
                    : fourCC == DDSPF_DXT3.fourCC ? GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
                    : fourCC == DDSPF_DXT5.fourCC ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
                                                  : GL_NONE;
    const GLuint blockSize = format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? 8 : 16;

    const unsigned bufferSize = mipMapCount > 1 ? linearSize * 2 : linearSize;
    unsigned char* buffer = new unsigned char[bufferSize];
    
    fs.read((char*)buffer, bufferSize);

    glBindTexture(GL_TEXTURE_2D, m_TextureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    unsigned bufferOffset = 0;
    unsigned currWidth = width;
    unsigned currHeight = height;

    for (unsigned mipLevel = 0; mipLevel < 1; ++mipLevel)
    {
        unsigned imageSize = ((currWidth + 3) / 4) * ((currHeight + 3) / 4) * blockSize;

        glCompressedTexImage2D(GL_TEXTURE_2D, mipLevel, format, currWidth, currHeight, 0, imageSize, buffer + bufferOffset);

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
    const GLuint format = fourCC == DDSPF_DXT1.fourCC   ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
                    : fourCC == DDSPF_DXT3.fourCC ? GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
                    : fourCC == DDSPF_DXT5.fourCC ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
                                                  : GL_NONE;

    const GLuint blockSize = format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? 8 : 16;

    unsigned bufferSize = mipCount > 1 ? linearSize * 2 : linearSize;
    unsigned char* buffer = new unsigned char[bufferSize];

    fs.read((char*)buffer, bufferSize);

    glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureHandle);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    for (unsigned i = 0; i < 6; ++i)
    {
        unsigned offset = 0;
        unsigned currWidth = width;
        unsigned currHeight = height;

        for (unsigned mipLevel = 0; mipLevel < mipCount; ++mipLevel)
        {
            unsigned imageSize = ((currWidth + 3) / 4) * ((currHeight + 3) / 4) * blockSize;

            glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mipLevel, format, currWidth, currHeight, 0, imageSize, buffer + offset);

            offset += imageSize;
            currWidth /= 2;
            currHeight /= 2;
        }
    }
    delete[] buffer;
}

void Texture::ImportTexture(const std::filesystem::path& filePath)
{
    stbi_set_flip_vertically_on_load(1);

    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* imageData = stbi_load(filePath.string().c_str(), &width, &height, &channels, 0);

    GLint internalFormat = channels == 3 ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : channels == 4 ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_NONE;
    GLenum format = internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? GL_RGB : internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT ? GL_RGBA : GL_NONE;

    glBindTexture(GL_TEXTURE_2D, m_TextureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);

    GLint imageSize;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &imageSize);
    char* imagePixels = new char[imageSize];

    glGetCompressedTexImage(GL_TEXTURE_2D, 0, imagePixels);

    // Setup DDS file header
    DirectX::DDS_HEADER header;
    header.size = sizeof(header);
    header.width = width;
    header.height = height;
    header.ddspf = internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? DirectX::DDSPF_DXT1 : internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT ? DirectX::DDSPF_DXT5 : DDS_PIXELFORMAT{};
    header.mipMapCount = 1;
    header.pitchOrLinearSize = imageSize;
    header.caps = DDS_SURFACE_FLAGS_TEXTURE;
    header.flags = DDS_HEADER_FLAGS_TEXTURE;

    m_FilePath.replace_extension(".dds");

    std::ofstream ofs = std::ofstream(m_FilePath, std::ios::binary);

    ofs.write((char*)&DDS_MAGIC, sizeof(DDS_MAGIC));
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

    glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureHandle);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int width = 0;
    int height = 0;
    int channels = 0;

    // Load first cubemap face; positive x
    unsigned char* imageData = stbi_load(faces[0].string().c_str(), &width, &height, &channels, 0);

    GLint internalFormat = channels == 3 ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : channels == 4 ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_NONE;
    GLenum format = internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? GL_RGB : internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT ? GL_RGBA : GL_NONE;

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);

    GLint imageSize;
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &imageSize);
    char* imagePixels = new char[imageSize];

    DirectX::DDS_HEADER header;
    header.size = sizeof(header);
    header.width = width;
    header.height = height;
    header.ddspf = internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? DirectX::DDSPF_DXT1 : internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT ? DirectX::DDSPF_DXT5 : DDS_PIXELFORMAT{};
    header.mipMapCount = 1;
    header.pitchOrLinearSize = imageSize;
    header.caps = DDS_SURFACE_FLAGS_CUBEMAP;
    header.flags = DDS_HEADER_FLAGS_TEXTURE;

    glGetCompressedTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, imagePixels);

    stbi_image_free(imageData);

    m_FilePath.replace_extension(".dds");

    std::ofstream ofs = std::ofstream(m_FilePath, std::ios::binary);

    ofs.write((char*)&DDS_MAGIC, sizeof(DDS_MAGIC));
    ofs.write((char*)&header, sizeof(header));
    ofs.write(imagePixels, imageSize);

    // Load other cubemap faces
    for (std::size_t i = 1; i < faces.size(); ++i)
    {
        imageData = stbi_load(faces[i].string().c_str(), &width, &height, &channels, 0);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (unsigned)i, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);

        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (unsigned)i, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &imageSize);

        glGetCompressedTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (unsigned)i, 0, imagePixels);

        stbi_image_free(imageData);

        ofs.write(imagePixels, imageSize);
    }

    ofs.close();
    delete[] imagePixels;
    m_LoadStatus = ResourceStatus::Loaded;
}

GLuint Texture::GetHandle() const
{
    return m_TextureHandle;
}
