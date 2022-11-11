#pragma once
#include <gl/glew.h>
#include <stb_truetype.h>

#include "MathDefs.h"
#include "ResourceBase.h"

struct FontCharacter
{
    /*
     * Glyph metrics:
     * --------------
     *
     *                       xmin                     xmax
     *                        |                         |
     *                        |<-------- width -------->|
     *                        |                         |
     *              |         +-------------------------+----------------- ymax
     *              |         |    ggggggggg   ggggg    |     ^        ^
     *              |         |   g:::::::::ggg::::g    |     |        |
     *              |         |  g:::::::::::::::::g    |     |        |
     *              |         | g::::::ggggg::::::gg    |     |        |
     *              |         | g:::::g     g:::::g     |     |        |
     *    offset_x -|-------->| g:::::g     g:::::g     |  offset_y    |
     *              |         | g:::::g     g:::::g     |     |        |
     *              |         | g::::::g    g:::::g     |     |        |
     *              |         | g:::::::ggggg:::::g     |     |        |
     *              |         |  g::::::::::::::::g     |     |      height
     *              |         |   gg::::::::::::::g     |     |        |
     *  baseline ---*---------|---- gggggggg::::::g-----*--------      |
     *            / |         |             g:::::g     |              |
     *     origin   |         | gggggg      g:::::g     |              |
     *              |         | g:::::gg   gg:::::g     |              |
     *              |         |  g::::::ggg:::::::g     |              |
     *              |         |   gg:::::::::::::g      |              |
     *              |         |     ggg::::::ggg        |              |
     *              |         |         gggggg          |              v
     *              |         +-------------------------+----------------- ymin
     *              |                                   |
     *              |------------- advance_x ---------->|
     */

    Vector2 m_BitmapSize;       // Size of bitmap
    Vector2 m_BitmapOffset;     // Offset of bitmap from glyph origin
    Vector2 m_TextureOffsetTL;  // Offset of glyph from texture
    Vector2 m_TextureOffsetBR;  // Offset of glyph from texture
    Vector2 m_Advance;          // Advancement of character to next one
};

class Font : public ResourceBase
{
public:

    Font(const std::string resourceName = "", const std::filesystem::path& filePath = "");
    ~Font();

    virtual void Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentElem) override;
    virtual void Unserialize(tinyxml2::XMLElement* currElem) override;
    virtual void Load() override;
    virtual void Unload() override;

    const FontCharacter& GetFontCharacterInfo(char c) const;
    unsigned GetAtlasSize() const;
    unsigned GetGlyphSize() const;
    GLuint GetTextureHandle() const;

private:

    FontCharacter m_FontCharacterInfos[128];
    GLuint m_FontTexture;
    unsigned m_AtlasSize;
    unsigned m_GlyphSize;
    unsigned m_SDFThreshold;
};