#pragma once
#include "CollisionMesh_3D.h"

// Plane colliders should always be used with a static rigidbody!
class PlaneCollider : public CollisionMesh_3D
{
public:
    PlaneCollider(GameObject* parentObject = nullptr, const float& radius = 0.5f);

    virtual ~PlaneCollider() = default;

    // getters
    std::vector<Vector3> GetVertices() const;
    float GetLength() const { return m_Length; }
    float GetWidth() const { return m_Width; }
    Vector3 GetNormal() const { return m_Normal; }

    // setters
    void SetLength(const float& len) { m_Length = len; }
    void SetWidth(const float& wid) { m_Width = wid; }
    void SetNormal(const Vector3& nrm) { m_Normal = nrm; }

    // funcs
    void DebugDraw() override;

private:
    float m_Length;
    float m_Width;
    Vector3 m_Normal;
};
