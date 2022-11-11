#pragma once
#include <GL/glew.h>

#include <map>

#include "CorePrerequisites.h"
#include "GeometryPass.h"

class Camera;

enum class DebugPrimitiveType
{
    Points = 0,
    Lines,
    LineLoops,
    Triangles,
    None
};

struct DebugDrawCommand
{
    int m_BaseVertex;
    int m_VertexCount;
    Color4 m_Color;
    DebugPrimitiveType m_Type;
};

enum class GizmoType
{
    TranslateX = 0x7FFF0000,
    TranslateY,
    TranslateZ,
    TranslateXZ,
    TranslateZY,
    TranslateYX,
    ScaleX,
    ScaleY,
    ScaleZ,
    RotateX,
    RotateY,
    RotateZ,
    None
};

enum class GizmoMode
{
    Translate,
    Scale,
    Rotate,
    None
};

static std::vector<Vector3> const gWireBoxMesh = { Vector3(-1.0f, 1.0f, 1.0f),   Vector3(-1.0f, -1.0f, 1.0f), Vector3(-1.0f, 1.0f, 1.0f),   Vector3(1.0f, 1.0f, 1.0f),   Vector3(-1.0f, -1.0f, 1.0f),
                                                   Vector3(1.0f, -1.0f, 1.0f),   Vector3(1.0f, -1.0f, 1.0f),  Vector3(1.0f, 1.0f, 1.0f),    Vector3(-1.0f, 1.0f, -1.0f), Vector3(-1.0f, 1.0f, 1.0f),
                                                   Vector3(1.0f, 1.0f, -1.0f),   Vector3(1.0f, 1.0f, 1.0f),   Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, -1.0f, 1.0f), Vector3(1.0f, -1.0f, -1.0f),
                                                   Vector3(1.0f, -1.0f, 1.0f),   Vector3(-1.0f, 1.0f, -1.0f), Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, 1.0f, -1.0f), Vector3(1.0f, 1.0f, -1.0f),
                                                   Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, -1.0f, -1.0f), Vector3(1.0f, -1.0f, -1.0f),  Vector3(1.0f, 1.0f, -1.0f) };

static std::vector<Vector3> const gSolidBoxMesh = { Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, -1.0f, 1.0f),  Vector3(-1.0f, 1.0f, 1.0f),   Vector3(1.0f, 1.0f, -1.0f),  Vector3(-1.0f, -1.0f, -1.0f),
                                                    Vector3(-1.0f, 1.0f, -1.0f),  Vector3(1.0f, -1.0f, 1.0f),   Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, -1.0f),
                                                    Vector3(1.0f, -1.0f, -1.0f),  Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, 1.0f, 1.0f),  Vector3(-1.0f, 1.0f, -1.0f),
                                                    Vector3(1.0f, -1.0f, 1.0f),   Vector3(-1.0f, -1.0f, 1.0f),  Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, 1.0f, 1.0f),  Vector3(-1.0f, -1.0f, 1.0f),
                                                    Vector3(1.0f, -1.0f, 1.0f),   Vector3(1.0f, 1.0f, 1.0f),    Vector3(1.0f, -1.0f, -1.0f),  Vector3(1.0f, 1.0f, -1.0f),  Vector3(1.0f, -1.0f, -1.0f),
                                                    Vector3(1.0f, 1.0f, 1.0f),    Vector3(1.0f, -1.0f, 1.0f),   Vector3(1.0f, 1.0f, 1.0f),    Vector3(1.0f, 1.0f, -1.0f),  Vector3(-1.0f, 1.0f, -1.0f),
                                                    Vector3(1.0f, 1.0f, 1.0f),    Vector3(-1.0f, 1.0f, -1.0f),  Vector3(-1.0f, 1.0f, 1.0f),   Vector3(1.0f, 1.0f, 1.0f),   Vector3(-1.0f, 1.0f, 1.0f),
                                                    Vector3(1.0f, -1.0f, 1.0f) };

class DebugPass
{
private:

    iVector2 m_ViewportSize;

    HShader m_DebugLineShader;
    HShader m_DebugGizmoShader;
    HShader m_DebugOutlineStencilShader;
    HShader m_DebugOutlineShader;

    GLuint m_DebugVertexArray;
    GLuint m_DebugVertexBuffer;

    std::map<unsigned, std::vector<DebugDrawCommand>> m_DebugDrawCommands;
    std::map<unsigned, std::vector<Vector3>> m_DebugDrawVertices;

    std::vector<Vector3> m_GridGeometry;

    std::vector<Vector2> m_SelectionBoxGeometry;
    bool m_SelectionBoxEnabled;

    std::vector<unsigned> m_SelectedObjects;
    GizmoMode m_GizmoMode;
    GizmoType m_HoveredGizmo;

public:

    DebugPass(iVector2 const& viewportSize);
    ~DebugPass();

    void Render(Camera& camera, GeometryPass& geomPass);

    void DrawDebug(unsigned layerId, std::vector<Vector3> const& geometry, Color4 color, DebugPrimitiveType type);
    void DrawCube(unsigned layerId, Color4 color);
    void DrawDebugBox(unsigned layerId, Color4 color);
    void DrawSelectionBox(float left, float right, float top, float bottom);

    void SetSelectedObjects(std::vector<unsigned> const& selectedObjects);
    void SetGizmoMode(GizmoMode mode);
    void SetHoveredGizmo(GizmoType gizmo);
};