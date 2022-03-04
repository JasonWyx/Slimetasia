#pragma once
#include "ConvexCollider.h"
#include "EdgeData.h"

class BoxCollider : public ConvexCollider
{
public:
    BoxCollider(GameObject* parentObject = nullptr, const Vector3& halfExtent = Vector3{0.5f, 0.5f, 0.5f});

    BoxCollider(GameObject* parentObject, const float& width, const float& height, const float& depth);

    ~BoxCollider() override = default;

    // getters
    float GetWidth() const { return m_HalfExtent.x * 2.f; }
    float GetHeight() const { return m_HalfExtent.y * 2.f; }
    float GetDepth() const { return m_HalfExtent.z * 2.f; }
    Vector3 GetHalfExtent() const { return m_HalfExtent; }
    Vector3 GetVertexPosition(const uint& index) const;
    EdgeData::Vertex GetVertex(const uint& index) const;
    EdgeData::Face GetFace(const uint& index) const;
    uint GetVertexCount() const { return 8u; }
    uint GetFaceCount() const { return 6u; }
    uint GetEdgeCount() const { return 12u; }

    // setters
    void SetWidth(const float& newWidth) { m_HalfExtent.x = newWidth / 2.f; }
    void SetHeight(const float& newHeight) { m_HalfExtent.y = newHeight / 2.f; }
    void SetDepth(const float& newDepth) { m_HalfExtent.z = newDepth / 2.f; }
    void SetHalfExtent(const Vector3& newHalf) { m_HalfExtent = newHalf; }

    // funcs
    void Initialize();

    void DebugDraw() override;

    void ComputeInertiaTensor(Matrix3& tensor, const float& mass) const override;

    Vector3 GetSupportPoint(const Vector3& dir) const override;

    Vector3 GetPointOnEdge(const Vector3& pt) const;

    bool Raycast(const Ray& ray, RaycastData_tmp& data) override;

    bool ContainsPoint(const Vector3& pt) override;

    void ComputeBounds(Vector3& min, Vector3& max) const override;

    friend struct ConvexPolygonVsConvexPolygon;
    friend struct SphereVsConvexPolygon;

    REFLECT()

protected:
    union
    {
        struct
        {
            float m_HalfWidth;
            float m_HalfHeight;
            float m_HalfDepth;
        };

        Vector3 m_HalfExtent;
    };

    EdgeData m_EdgeData;
};
