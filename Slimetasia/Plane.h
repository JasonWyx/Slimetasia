#pragma once
#include "CorePrerequisites.h"

namespace IntersectionType
{
    enum Type
    {
        Coplanar = 0,
        Outside,
        Inside,
        Overlaps,
        NotImplemented
    };
    static const char* Names[] = {"Coplanar", "Outside", "Inside", "Overlaps", "NotImplemented"};
}  // namespace IntersectionType

class Plane
{
public:
    Plane();

    Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2);

    Plane(const Vector3& normal, const Vector3& point);

    Plane(const Vector4& data);

    Plane(const Vector3& normal, const float& dist);

    Plane(const float& x, const float& y, const float& z, const float& d);

    ~Plane() = default;

    void DebugDraw(const float& size, const unsigned int& parentID) const;

    void DebugDraw(const float& x, const float& y, const unsigned int& parentID) const;

    // Create a plane from a triangle. The plane's normal should be normalized.
    void Set(const Vector3& p0, const Vector3& p1, const Vector3& p2);
    // Create a plane from a point and a normal. The plane's normal should be normalized.
    void Set(const Vector3& normal, const Vector3& point);

    // getters
    Vector4 GetData() const { return m_Data; }

    Vector3 GetNormal() const { return m_Normal; }

    float GetDistance() const { return m_Distance; }

    // setters
    void SetData(const Vector4& data) { m_Data = data; }

    void SetNormal(const Vector3& nrm) { m_Normal = nrm; }

    void SetDistance(const float& dist) { m_Distance = dist; }

    static Vector3 ProjectPointOnPlane(const Vector3& point, const Vector3& normal, float planeDistance);

    static IntersectionType::Type PlaneSphere(const Vector4& plane, const Vector3& sphereCenter, float sphereRadius);

    static IntersectionType::Type PlaneAabb(const Vector4& plane, const Vector3& aabbMin, const Vector3& aabbMax);

private:
    union
    {
        union
        {
            Vector3 m_Normal;
            float m_Distance;
        };

        Vector4 m_Data;
    };
};