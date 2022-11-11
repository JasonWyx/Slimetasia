#pragma once
#include <vector>

#include "Utility.h"

class EdgeData
{
public:

    struct Vertex
    {
        Vertex(const int& index);

        int m_VertexPointIndex;
    };

    struct Edge
    {
        uint m_FirstVertex;
        uint m_SecondVertex;

        Vector3 GetEdgeNormal() const;
    };

    struct Face
    {
        Face(const std::vector<Vertex>& vertices);

        std::vector<Vertex> m_FaceVertices;

        Vector3 GetFaceNormal() const;
    };

    size_t GetVertexCount() const { return m_Vertices.size(); }
    size_t GetEdgeCount() const { return m_Edges.size(); }
    size_t GetFaceCount() const { return m_Faces.size(); }

    Vertex GetVertex(const uint& index) const { return m_Vertices[index]; }
    Edge GetEdge(const uint& index) const { return m_Edges[index]; }
    Face GetFace(const uint& index) const { return m_Faces[index]; }

    void AddVertex(const uint& index);
    void AddFace(const std::vector<Vertex>& verts);
    void Initialize();
    void ClearData();

private:

    std::vector<Vertex> m_Vertices;
    std::vector<Edge> m_Edges;
    std::vector<Face> m_Faces;
};
