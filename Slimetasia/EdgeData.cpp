#include "EdgeData.h"

EdgeData::Face::Face(const std::vector<Vertex>& vertices)
    : m_FaceVertices(vertices)
{
}

Vector3 EdgeData::Face::GetFaceNormal() const
{
    return Vector3();
}

EdgeData::Vertex::Vertex(const int& index)
    : m_VertexPointIndex(index)
{
}

void EdgeData::AddVertex(const uint& index)
{
    m_Vertices.emplace_back(index);
}

void EdgeData::AddFace(const std::vector<Vertex>& verts)
{
    m_Faces.emplace_back(verts);
}

void EdgeData::Initialize()
{
    for (auto i = 0u; i < m_Vertices.size(); i++)
    {
    }
}

void EdgeData::ClearData()
{
    m_Vertices.clear();
    m_Faces.clear();
    m_Edges.clear();
}

Vector3 EdgeData::Edge::GetEdgeNormal() const
{
    return Vector3();
}
