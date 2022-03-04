#pragma once
#include "CollisionMesh_3D.h"

class CapsuleCollider : public CollisionMesh_3D
{
public:
    CapsuleCollider(GameObject* parentObject = nullptr, const float& radius = 0.5f, const float& half_height = 0.5f);

    virtual ~CapsuleCollider() = default;

    // getters
    float GetRadius() const { return m_radius; }

    float GetHalfHeight() const { return m_HalfHeight; }

    // setters
    void SetRadius(const float& new_rad) { m_radius = new_rad; }

    void SetHalfHeight(const float& new_halfheight) { m_HalfHeight = new_halfheight; }

    // funcs
    void DebugDraw() override;

    bool ContainsPoint(const Vector3& pt) override;

    bool Raycast(const Ray& ray, RaycastData_tmp& data) override;

    void ComputeInertiaTensor(Matrix3& tensor, const float& mass) const override;

    Vector3 GetClosestPoint(const Vector3& inp) const;

    friend struct SphereVsCapsule;
    friend struct CapsuleVsCapsule;
    friend struct CapsuleVsConvexPolygon;

    REFLECT()

private:
    float m_HalfHeight;
};
