#include "TextRenderer.h"

#include <vector>

#include "GameObject.h"
#include "ResourceManager.h"

REFLECT_INIT(TextRenderer)
REFLECT_PARENT(IComponent)
REFLECT_PROPERTY(m_Text)
REFLECT_PROPERTY(m_Font)
REFLECT_PROPERTY(m_FontSize)
REFLECT_PROPERTY(m_FontColor)
REFLECT_PROPERTY(m_OutlineEnabled)
REFLECT_PROPERTY(m_OutlineWidth)
REFLECT_PROPERTY(m_OutlineColor)
REFLECT_PROPERTY(m_AnchorPoint)
REFLECT_PROPERTY(m_FaceCamera)
REFLECT_END()

TextRenderer::TextRenderer(GameObject* owner)
    : IComponent(owner, "TextRenderer")
    , m_Transform(owner->GetComponent<Transform>())
    , m_Text("abcdefghijklmnopqrstuvwxyz\nABCDEFGHIJKLMNOPQRSTUVWXYZ\n0123456789")
    , m_Font(ResourceManager::Instance().GetResource<Font>("arial"))
    , m_FontSize(1.0f)
    , m_FontColor(Color4(1.0f))
    , m_OutlineEnabled(false)
    , m_OutlineWidth(1.0f)
    , m_OutlineColor(0.0f)
    , m_FaceCamera(false)
    , m_AnchorPoint(TextAnchorPoint::eTextAnchorPoint_TopLeft)
    , m_PrevAnchorPoint(m_AnchorPoint)
{
    glCreateVertexArrays(1, &m_VertexArray);

    glEnableVertexArrayAttrib(m_VertexArray, 0);
    glVertexArrayAttribBinding(m_VertexArray, 0, 0);
    glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(m_VertexArray, 1);
    glVertexArrayAttribBinding(m_VertexArray, 1, 0);
    glVertexArrayAttribFormat(m_VertexArray, 1, 2, GL_FLOAT, GL_FALSE, sizeof(Vector3));

    m_OutlineColor = Color4(0.f, 0.f, 0.f, 1.0f);
    // GenerateGeometryData();
}

TextRenderer::~TextRenderer()
{
    glDeleteVertexArrays(1, &m_VertexArray);
    glDeleteBuffers(1, &m_VertexBuffer);
}

void TextRenderer::GenerateGeometryData()
{
    Vector2 charPos;
    unsigned charCount = 0;
    float fontSize = m_FontSize;
    float textWidth = 0;
    float textHeight = static_cast<float>(m_Font->GetGlyphSize());
    float maxTextWidth = 0;
    unsigned maxFontHeight = m_Font->GetGlyphSize();

    // Calculate text box dimensions
    for (const char& c : m_Text)
    {
        if (c == '\n')
        {
            maxTextWidth = std::max(textWidth, maxTextWidth);
            textHeight += maxFontHeight;
            textWidth = 0;
        }
        else
        {
            textWidth += m_Font->GetFontCharacterInfo(c).m_Advance.x;
        }
    }
    maxTextWidth = std::max(textWidth, maxTextWidth);

    // textWidth = maxTextWidth;

    // Set text offset
    switch (m_AnchorPoint)
    {
        case TextAnchorPoint::eTextAnchorPoint_TopLeft:
        {
            charPos.y = -(float)maxFontHeight;
            break;
        }
        case TextAnchorPoint::eTextAnchorPoint_Top:
        {
            charPos.y = -(float)maxFontHeight;
            charPos.x = -maxTextWidth / 2;
            break;
        }
        case TextAnchorPoint::eTextAnchorPoint_TopRight:
        {
            charPos.y = -(float)maxFontHeight;
            charPos.x = -maxTextWidth;
            break;
        }
        case TextAnchorPoint::eTextAnchorPoint_Left:
        {
            charPos.y = -(float)maxFontHeight + textHeight / 2;
            break;
        }
        case TextAnchorPoint::eTextAnchorPoint_Center:
        {
            charPos.x = -maxTextWidth / 2;
            charPos.y = -(float)maxFontHeight + textHeight / 2;
            break;
        }
        case TextAnchorPoint::eTextAnchorPoint_Right:
        {
            charPos.x = -maxTextWidth;
            charPos.y = -(float)maxFontHeight + textHeight / 2;
            break;
        }
        case TextAnchorPoint::eTextAnchorPoint_BottomLeft:
        {
            charPos.y = -(float)maxFontHeight + textHeight;
            break;
        }
        case TextAnchorPoint::eTextAnchorPoint_Bottom:
        {
            charPos.x = -maxTextWidth / 2;
            charPos.y = -(float)maxFontHeight + textHeight;
            break;
        }
        case TextAnchorPoint::eTextAnchorPoint_BottomRight:
        {
            charPos.x = -maxTextWidth;
            charPos.y = -(float)maxFontHeight + textHeight;
            break;
        }

    }  // switch(m_AnchorPoint)

    // Generate geometry
    struct TextVertex
    {
        Vector3 position;
        Vector2 texCoord;
    };

    std::vector<unsigned> indices;
    std::vector<TextVertex> vertices;

    for (const char& c : m_Text)
    {
        if (c == '\n')
        {
            charPos.y -= maxFontHeight;
            switch (m_AnchorPoint)
            {
                case TextAnchorPoint::eTextAnchorPoint_TopRight:
                case TextAnchorPoint::eTextAnchorPoint_Right:
                case TextAnchorPoint::eTextAnchorPoint_BottomRight: charPos.x = -maxTextWidth; break;
                case TextAnchorPoint::eTextAnchorPoint_Top:
                case TextAnchorPoint::eTextAnchorPoint_Bottom:
                case TextAnchorPoint::eTextAnchorPoint_Center: charPos.x = -maxTextWidth / 2; break;
                case TextAnchorPoint::eTextAnchorPoint_TopLeft:
                case TextAnchorPoint::eTextAnchorPoint_Left:
                case TextAnchorPoint::eTextAnchorPoint_BottomLeft: charPos.x = 0; break;
            }
        }

        const FontCharacter& charInfo = m_Font->GetFontCharacterInfo(c);
        const float& charTop = charInfo.m_BitmapOffset.y;
        const float& charLeft = charInfo.m_BitmapOffset.x;
        const float& charWidth = charInfo.m_BitmapSize.x;
        const float& charHeight = charInfo.m_BitmapSize.y;

        float fltMaxFontHeight = static_cast<float>(maxFontHeight);

        // Top Left
        vertices.emplace_back(TextVertex{Vector3(charPos.x + charLeft, charPos.y + charTop, 0.f) / fltMaxFontHeight * m_FontSize, Vector2(charInfo.m_TextureOffsetTL.x, charInfo.m_TextureOffsetTL.y)});

        // Bottom left
        vertices.emplace_back(
            TextVertex{Vector3(charPos.x + charLeft, charPos.y + charTop - charHeight, 0.f) / fltMaxFontHeight * m_FontSize, Vector2(charInfo.m_TextureOffsetTL.x, charInfo.m_TextureOffsetBR.y)});

        // Bottom right
        vertices.emplace_back(TextVertex{Vector3(charPos.x + charLeft + charWidth, charPos.y + charTop - charHeight, 0.f) / fltMaxFontHeight * m_FontSize,
                                         Vector2(charInfo.m_TextureOffsetBR.x, charInfo.m_TextureOffsetBR.y)});

        // Top right
        vertices.emplace_back(
            TextVertex{Vector3(charPos.x + charLeft + charWidth, charPos.y + charTop, 0.f) / fltMaxFontHeight * m_FontSize, Vector2(charInfo.m_TextureOffsetBR.x, charInfo.m_TextureOffsetTL.y)});

        indices.insert(indices.end(), {
                                          static_cast<unsigned>(charCount * 4 + 0),
                                          static_cast<unsigned>(charCount * 4 + 1),
                                          static_cast<unsigned>(charCount * 4 + 2),
                                          static_cast<unsigned>(charCount * 4 + 2),
                                          static_cast<unsigned>(charCount * 4 + 3),
                                          static_cast<unsigned>(charCount * 4 + 0),
                                      });

        ++charCount;
        charPos += charInfo.m_Advance;
    }

    m_TotalIndices = static_cast<unsigned int>(indices.size());

    // Clean up first
    glDeleteBuffers(1, &m_VertexBuffer);
    glDeleteBuffers(1, &m_IndexBuffer);

    // Generate buffers and store data
    glCreateBuffers(1, &m_VertexBuffer);
    glCreateBuffers(1, &m_IndexBuffer);

    glNamedBufferStorage(m_VertexBuffer, vertices.size() * sizeof(vertices[0]), vertices.data(), 0);
    glVertexArrayVertexBuffer(m_VertexArray, 0, m_VertexBuffer, 0, sizeof(TextVertex));

    glNamedBufferStorage(m_IndexBuffer, indices.size() * sizeof(indices[0]), indices.data(), 0);
    glVertexArrayElementBuffer(m_VertexArray, m_IndexBuffer);
}

GLuint TextRenderer::GetVertexArrayObject()
{
    size_t textHash = std::hash<std::string>()(m_Text);

    if (textHash != m_TextHash || m_AnchorPoint != m_PrevAnchorPoint || m_Font != m_PrevFont)
    {
        m_PrevFont = m_Font;
        m_TextHash = textHash;
        m_PrevAnchorPoint = m_AnchorPoint;
        GenerateGeometryData();
    }

    return m_VertexArray;
}

unsigned TextRenderer::GetTotalIndices()
{
    return m_TotalIndices;
}

void TextRenderer::OnActive()
{
    if (m_OwnerObject->GetParentLayer()) m_OwnerObject->GetParentLayer()->GetRenderLayer().AddTextRenderer(this);
}

void TextRenderer::OnInactive()
{
    if (m_OwnerObject->GetParentLayer()) m_OwnerObject->GetParentLayer()->GetRenderLayer().RemoveTextRenderer(this);
}
