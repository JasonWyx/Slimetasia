/******************************************************************************/
/*!
All content © 2017 DigiPen Institute of Technology Singapore. All Rights Reserved

File Name: Font.hpp
Author(s): Wang Yuxuan Jason
Contributions: 100%

Brief Description:

This file is the implementation file that handles font system

*/
/******************************************************************************/
#define STB_TRUETYPE_IMPLEMENTATION
#include "Font.h"

unsigned constexpr PADDING = 5;

Font::Font(const std::string resourceName, const std::filesystem::path& filePath)
    : ResourceBase(resourceName, filePath)
    , m_FontCharacterInfos()
    , m_FontTexture()
    , m_AtlasSize(1024)
    , m_GlyphSize(64)
    , m_SDFThreshold(180)
{
}

Font::~Font()
{
    Unload();
}

void Font::Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentElem)
{
    tinyxml2::XMLElement* resourceElem = doc->NewElement("Font");
    tinyxml2::XMLElement* nameElem = doc->NewElement("Name");
    tinyxml2::XMLElement* filepathElem = doc->NewElement("Filepath");

    resourceElem->SetAttribute("GUID", static_cast<int64_t>(m_GUID));
    nameElem->SetText(m_Name.c_str());
    filepathElem->SetText(m_FilePath.string().c_str());

    parentElem->InsertEndChild(resourceElem);
    resourceElem->InsertEndChild(nameElem);
    resourceElem->InsertEndChild(filepathElem);
}

void Font::Unserialize(tinyxml2::XMLElement* currElem)
{
    m_GUID = (ResourceGUID)currElem->Int64Attribute("GUID");
    m_Name = currElem->FirstChildElement("Name")->GetText();
    m_FilePath = currElem->FirstChildElement("Filepath")->GetText();
}

void Font::Load()
{
    stbtt_fontinfo font;

    std::ifstream file(m_FilePath.string(), std::ios::binary | std::ios::ate);
    std::vector<unsigned char> ttfData(file.tellg());
    file.seekg(0);
    file.read((char*)ttfData.data(), ttfData.size());

    if (!stbtt_InitFont(&font, ttfData.data(), 0))
    {
        std::cout << "Failed to initialize font resource " << m_FilePath << std::endl;
        return;
    }

    const int textureSize = m_AtlasSize;
    const int glyphSize = m_GlyphSize;
    int currX = 0;
    int currY = 0;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_FontTexture);
    glTextureStorage2D(m_FontTexture, 1, GL_R8, textureSize, textureSize);
    glTextureParameteri(m_FontTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_FontTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    float scale = stbtt_ScaleForPixelHeight(&font, 56);

    for (unsigned char character = 32; character < (32 + 96); ++character)
    {
        int glyphIndex = stbtt_FindGlyphIndex(&font, character);

        int advanceWidth, leftSideBearing;
        stbtt_GetGlyphHMetrics(&font, glyphIndex, &advanceWidth, &leftSideBearing);

        int width = 0, height = 0, xoff = 0, yoff = 0;
        unsigned char* glyphPixels = stbtt_GetGlyphSDF(&font, scale, glyphIndex, 4, m_SDFThreshold, m_SDFThreshold / 5.0f, &width, &height, &xoff, &yoff);

        m_FontCharacterInfos[character].m_BitmapOffset = Vector2(static_cast<float>(xoff), static_cast<float>(-yoff));
        m_FontCharacterInfos[character].m_BitmapSize = Vector2(static_cast<float>(width), static_cast<float>(height));
        m_FontCharacterInfos[character].m_TextureOffsetTL = Vector2(static_cast<float>(currX), static_cast<float>(currY)) / static_cast<float>(textureSize);
        m_FontCharacterInfos[character].m_TextureOffsetBR = Vector2(static_cast<float>(currX + width), static_cast<float>(currY + height)) / static_cast<float>(textureSize);
        m_FontCharacterInfos[character].m_Advance = Vector2(static_cast<float>((advanceWidth + leftSideBearing)) * scale, 0);

        glTextureSubImage2D(m_FontTexture, 0, currX, currY, width, height, GL_RED, GL_UNSIGNED_BYTE, glyphPixels);

        currX += glyphSize;
        if (currX >= textureSize)
        {
            currX = 0;
            currY += glyphSize;
        }

        stbtt_FreeSDF(glyphPixels, nullptr);
    }

    file.close();
}

void Font::Unload()
{
    if (m_FontTexture)
    {
        glDeleteTextures(1, &m_FontTexture);
    }
}

const FontCharacter& Font::GetFontCharacterInfo(char c) const
{
    return m_FontCharacterInfos[c];
}

unsigned Font::GetAtlasSize() const
{
    return m_AtlasSize;
}

unsigned Font::GetGlyphSize() const
{
    return m_GlyphSize;
}

GLuint Font::GetTextureHandle() const
{
    return m_FontTexture;
}
