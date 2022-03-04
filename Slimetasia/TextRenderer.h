#pragma once
#include <string>

#include "Font.h"
#include "IComponent.h"
#include "Transform.h"
#include "smart_enums.h"

enum class AnchorPoint
{
    TopLeft,
    Top,
    TopRight,
    Right,
    BottomRight,
    Bottom,
    BottomLeft,
    Left,
    Center
};

#define TextAnchorPoint_List(m)                                                                                                                                           \
    m(TextAnchorPoint, TopLeft) m(TextAnchorPoint, Top) m(TextAnchorPoint, TopRight) m(TextAnchorPoint, Right) m(TextAnchorPoint, BottomRight) m(TextAnchorPoint, Bottom) \
        m(TextAnchorPoint, BottomLeft) m(TextAnchorPoint, Left) m(TextAnchorPoint, Center)

SMARTENUM_DEFINE_ENUM(TextAnchorPoint, TextAnchorPoint_List)
SMARTENUM_DEFINE_NAMES(TextAnchorPoint, TextAnchorPoint_List)
SMARTENUM_DEFINE_GET_VALUE_FROM_STRING(TextAnchorPoint)
SMARTENUM_DEFINE_GET_ALL_VALUES_IN_STRING(TextAnchorPoint)
REFLECT_ENUM(TextAnchorPoint)

class TextRenderer : public IComponent
{
public:
    TextRenderer(GameObject* owner);
    ~TextRenderer();

    void OnActive() override;
    void OnInactive() override;

private:
    // Owner transform
    std::size_t m_TextHash;

    GLuint m_VertexArray;
    GLuint m_VertexBuffer;
    GLuint m_IndexBuffer;
    unsigned m_TotalIndices;
    TextAnchorPoint m_PrevAnchorPoint;

    void GenerateGeometryData();

public:
    // Text rendering related stuff
    Transform* m_Transform;
    std::string m_Text;
    HFont m_Font;
    HFont m_PrevFont;
    float m_FontSize;
    Color4 m_FontColor;
    bool m_OutlineEnabled;
    float m_OutlineWidth;
    Color4 m_OutlineColor;
    bool m_FaceCamera;
    TextAnchorPoint m_AnchorPoint;

    GLuint GetVertexArrayObject();
    unsigned GetTotalIndices();

    REFLECT();
};