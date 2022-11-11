#pragma once
#include "CollisionMesh_3D.h"

class SphereCollider : public CollisionMesh_3D
{
public:

    SphereCollider(GameObject* parentObject = nullptr, const float& radius = 0.5f);

    virtual ~SphereCollider() = default;

    // getters
    float GetRadius() const { return m_radius; }

    // setters
    void SetRadius(const float& new_rad) { m_radius = new_rad; }

    // funcs
    void DebugDraw() override;

    bool ContainsPoint(const Vector3& pt) override;

    bool Raycast(const Ray& ray, RaycastData_tmp& data) override;

    void ComputeInertiaTensor(Matrix3& tensor, const float& mass) const override;

    void ComputeBounds(Vector3& min, Vector3& max) const override;

    friend struct SphereVsConvexPolygon;
    friend struct SphereVsSphere;
    friend struct SphereVsCapsule;

    REFLECT()

private:
};
