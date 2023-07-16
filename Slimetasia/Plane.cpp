#include "Plane.h"

#include "Layer.h"
#include "Ray.h"
#include "Renderer.h"

Plane::Plane()
    : m_Data { 0.f, 0.f, 0.f, 0.f }
{
}

Plane::Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2)
    : Plane()
{
    Set(p0, p1, p2);
}

Plane::Plane(const Vector3& normal, const Vector3& point)
    : Plane()
{
    Set(normal, point);
}

Plane::Plane(const Vector3& normal, const float& dist)
    : m_Data { normal, dist }
{
}

Plane::Plane(const Vector4& data)
    : m_Data { data }
{
}

Plane::Plane(const float& x, const float& y, const float& z, const float& d)
    : m_Data { x, y, z, d }
{
}

void Plane::DebugDraw(const float& size, const unsigned int& parentID) const
{
    DebugDraw(size, size, parentID);
}

void Plane::DebugDraw(const float& x, const float& y, const unsigned int& parentID) const
{
#ifdef USE_VULKAN
    auto currentLayerID = 0U;  // todo
#else
    auto currentLayerID = Renderer::Instance().GetCurrentEditorLayer()->GetId();
#endif  // USE_VULKAN

    if (parentID != currentLayerID)
    {
        return;
    }

    std::vector<Vector3> pts;

    // WarnIf(ShowDebugDrawWarnings, "Assignment2: Required function un-implemented");
    auto pt = Vector3 { m_Data.x * m_Data.w, m_Data.y * m_Data.w, m_Data.z * m_Data.w };
    auto v = Vector3 { 0.f, 0.f, 1.f }.Cross(pt);
    if (v.SquareLength() == 0.f) v = pt.Cross(Vector3 { 1.f, 0.f, 0.f });
    v = v.Normalized();
    auto w = pt.Cross(v);
    w = w.Normalized();
    auto iv = x * v;
    auto jw = y * w;

    pts.emplace_back(pt + iv + jw);
    pts.emplace_back(pt + iv - jw);
    pts.emplace_back(pt + iv + jw);
    pts.emplace_back(pt - iv + jw);
    pts.emplace_back(pt - iv - jw);
    pts.emplace_back(pt + iv - jw);
    pts.emplace_back(pt - iv - jw);
    pts.emplace_back(pt - iv + jw);

#ifdef USE_VULKAN
#else
    Renderer::Instance().DrawDebug(currentLayerID, pts, Color4(0.0f, 1.0f, 1.0f, 1.0f), DebugPrimitiveType::Lines);
#endif  // USE_VULKAN

    DrawRay(Ray { pt, m_Normal }, 1.f, parentID, Color4(0.0f, 1.0f, 1.0f, 1.0f));
}

void Plane::Set(const Vector3& p0, const Vector3& p1, const Vector3& p2)
{
    auto normal = (p1 - p0).Cross(p2 - p0);
    normal = normal.Normalized();
    m_Data.x = normal.x;
    m_Data.y = normal.y;
    m_Data.z = normal.z;
    m_Data.w = p0.Dot(normal);
}

void Plane::Set(const Vector3& normal, const Vector3& point)
{
    auto tmp = normal.Normalized();
    m_Data.x = normal.x;
    m_Data.y = normal.y;
    m_Data.z = normal.z;
    m_Data.w = point.Dot(normal);
}

Vector3 Plane::ProjectPointOnPlane(const Vector3& point, const Vector3& normal, float planeDistance)
{
    auto w = point.Dot(normal) - planeDistance;
    return point - w * normal;
}

IntersectionType::Type Plane::PlaneSphere(const Vector4& plane, const Vector3& sphereCenter, float sphereRadius)
{
    auto norm = Vector3 { plane.x, plane.y, plane.z };
    auto p_prime = Plane::ProjectPointOnPlane(sphereCenter, norm, plane.w);
    auto rad_sq = sphereRadius * sphereRadius;
    auto len = (sphereCenter - p_prime).SquareLength();
    if (len <= rad_sq) return IntersectionType::Overlaps;
    auto dist = sphereCenter.Dot(norm) - plane.w;
    return dist < sphereRadius ? IntersectionType::Outside : IntersectionType::Inside;
}

IntersectionType::Type Plane::PlaneAabb(const Vector4& plane, const Vector3& aabbMin, const Vector3& aabbMax)
{
    Vector3 he { (aabbMax.x - aabbMin.x) / 2.f, (aabbMax.y - aabbMin.y) / 2.f, (aabbMax.z - aabbMin.z) / 2.f };
    auto center = aabbMin + he;
    he.x *= plane.x >= 0.f ? 1.f : -1.f;
    he.y *= plane.y >= 0.f ? 1.f : -1.f;
    he.z *= plane.z >= 0.f ? 1.f : -1.f;
    auto d = Vector3 { plane.x, plane.y, plane.z }.Dot(he);
    d *= d < 0.f ? -1.f : 1.f;
    return Plane::PlaneSphere(plane, center, d);
}
