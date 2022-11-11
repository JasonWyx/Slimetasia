#include "AABB.h"

#include "GameObject.h"
#include "Layer.h"
#include "MeshRenderer.h"
#include "Renderer.h"
#include "RigidbodyComponent.h"
#include "Transform.h"

AABB::AABB()
{
    // set the aabb to an initial bad value (where the min is smaller than the max)
    m_Min.x = m_Min.y = m_Min.z = MAX_FLOAT;
    m_Max.x = m_Max.y = m_Max.z = MIN_FLOAT;
}

AABB::AABB(const Vector3& min, const Vector3& max)
{
    m_Min = min;
    m_Max = max;
}

AABB::AABB(RigidbodyComponent* rigidbody)
{
    rigidbody->GetBounds(m_Min, m_Max);
}

AABB AABB::BuildFromCenterAndHalfExtents(const Vector3& center, const Vector3& halfExtents)
{
    return AABB(center - halfExtents, center + halfExtents);
}

float AABB::GetVolume() const
{
    /******Student:Assignment2******/
    // Return the aabb's volume
    // Warn("Assignment2: Required function un-implemented");
    Vector3 u { m_Max.x - m_Min.x, 0.f, 0.f };
    Vector3 v { 0.f, m_Max.y - m_Min.y, 0.f };
    Vector3 w { 0.f, 0.f, m_Max.z - m_Min.z };

    return abs(u.Cross(v).Dot(w));
}

float AABB::GetSurfaceArea() const
{
    /******Student:Assignment2******/
    // Return the aabb's surface area
    // Warn("Assignment2: Required function un-implemented");
    Vector3 dimens(m_Max - m_Min);

    return 2.f * (dimens.x * dimens.y + dimens.x * dimens.z + dimens.y * dimens.z);
}

bool AABB::Contains(const AABB& aabb) const
{
    /******Student:Assignment2******/
    // Return if aabb is completely contained in this
    // Warn("Assignment2: Required function un-implemented");
    return (aabb.m_Min.x <= m_Max.x && aabb.m_Min.x >= m_Min.x && aabb.m_Max.x <= m_Max.x && aabb.m_Max.x >= m_Min.x && aabb.m_Min.y <= m_Max.y && aabb.m_Min.y >= m_Min.y && aabb.m_Max.y <= m_Max.y &&
            aabb.m_Max.y >= m_Min.y && aabb.m_Min.z <= m_Max.z && aabb.m_Min.z >= m_Min.z && aabb.m_Max.z <= m_Max.z && aabb.m_Max.z >= m_Min.z);
}

void AABB::Expand(const Vector3& point)
{
    for (auto i = 0u; i < 3; ++i)
    {
        m_Min[i] = std::min(m_Min[i], point[i]);
        m_Max[i] = std::max(m_Max[i], point[i]);
    }
}

AABB AABB::Combine(const AABB& lhs, const AABB& rhs)
{
    AABB result;
    for (auto i = 0u; i < 3; ++i)
    {
        result.m_Min[i] = std::min(lhs.m_Min[i], rhs.m_Min[i]);
        result.m_Max[i] = std::max(lhs.m_Max[i], rhs.m_Max[i]);
    }
    return result;
}

bool AABB::Compare(const AABB& rhs, float epsilon) const
{
    float pos1Diff = (m_Min - rhs.m_Min).Length();
    float pos2Diff = (m_Max - rhs.m_Max).Length();

    return pos1Diff < epsilon && pos2Diff < epsilon;
}

void AABB::Transform(const Vector3& scale, const Matrix3& rotation, const Vector3& translation)
{
    Vector3 center((m_Max + m_Min) / 2.f);
    Vector3 extent(m_Max - center);
    extent *= scale;
    const Vector3 newextent(abs(rotation[0]) * extent.x + abs(rotation[1]) * extent.y + abs(rotation[2]) * extent.z,
                            abs(rotation[3]) * extent.x + abs(rotation[4]) * extent.y + abs(rotation[5]) * extent.z,
                            abs(rotation[6]) * extent.x + abs(rotation[7]) * extent.y + abs(rotation[8]) * extent.z);

    center *= scale;
    Vector3 newcenter(rotation[0] * center.x + rotation[1] * center.y + rotation[2] * center.z, rotation[3] * center.x + rotation[4] * center.y + rotation[5] * center.z,
                      rotation[6] * center.x + rotation[7] * center.y + rotation[8] * center.z);

    newcenter += translation;
    m_Max = newcenter + newextent;
    m_Min = newcenter - newextent;
}

Vector3 AABB::GetMin() const
{
    return m_Min;
}

Vector3 AABB::GetMax() const
{
    return m_Max;
}

Vector3 AABB::GetCenter() const
{
    return (m_Min + m_Max) * 0.5f;
}

Vector3 AABB::GetHalfSize() const
{
    return (m_Max - m_Min) * 0.5f;
}

void AABB::DebugDraw(const unsigned int& parentID) const
{
#if EDITOR
    auto currentLayerID = Renderer::Instance().GetCurrentEditorLayer()->GetId();
    if (parentID != currentLayerID) return;

    std::vector<Vector3> pts;

    pts.emplace_back(m_Max);
    pts.emplace_back(m_Max.x, m_Max.y, m_Min.z);

    pts.emplace_back(m_Max);
    pts.emplace_back(m_Max.x, m_Min.y, m_Max.z);

    pts.emplace_back(m_Max);
    pts.emplace_back(m_Min.x, m_Max.y, m_Max.z);

    pts.emplace_back(m_Min.x, m_Min.y, m_Max.z);
    pts.emplace_back(m_Max.x, m_Min.y, m_Max.z);

    pts.emplace_back(m_Max.x, m_Min.y, m_Min.z);
    pts.emplace_back(m_Max.x, m_Min.y, m_Max.z);

    pts.emplace_back(m_Max.x, m_Max.y, m_Min.z);
    pts.emplace_back(m_Max.x, m_Min.y, m_Min.z);

    pts.emplace_back(m_Min);
    pts.emplace_back(m_Max.x, m_Min.y, m_Min.z);

    pts.emplace_back(m_Min);
    pts.emplace_back(m_Min.x, m_Min.y, m_Max.z);

    pts.emplace_back(m_Min);
    pts.emplace_back(m_Min.x, m_Max.y, m_Min.z);

    pts.emplace_back(m_Min.x, m_Max.y, m_Max.z);
    pts.emplace_back(m_Min.x, m_Min.y, m_Max.z);

    pts.emplace_back(m_Min.x, m_Max.y, m_Min.z);
    pts.emplace_back(m_Min.x, m_Max.y, m_Max.z);

    pts.emplace_back(m_Min.x, m_Max.y, m_Min.z);
    pts.emplace_back(m_Max.x, m_Max.y, m_Min.z);

    Renderer::Instance().DrawDebug(currentLayerID, pts, Color4(0.0f, 1.0f, 1.0f, 1.0f), DebugPrimitiveType::Lines);
#endif
}

bool AABB::AABBAABB(const Vector3& aabbMin0, const Vector3& aabbMax0, const Vector3& aabbMin1, const Vector3& aabbMax1)
{
    return !((aabbMin1.x > aabbMax0.x || aabbMin0.x > aabbMax1.x) || (aabbMin1.y > aabbMax0.y || aabbMin0.y > aabbMax1.y) || (aabbMin1.z > aabbMax0.z || aabbMin0.z > aabbMax1.z));
}

bool AABB::RayAabb(const Vector3& rayStart, const Vector3& rayDir, const Vector3& aabbMin, const Vector3& aabbMax, float& t)
{
    Vector4 plane_l { 1.f, 0.f, 0.f, aabbMin.x }, plane_bo { 0.f, 1.f, 0.f, aabbMin.y }, plane_f { 0.f, 0.f, 1.f, aabbMin.z };
    auto denom = 0.f;
    t = 0.f;
    auto tmax = FLT_MAX, t1 = 0.f, t2 = 0.f;
    // checking for x axis
    if (rayDir.x == 0.f)
    {
        if (aabbMin.x > rayStart.x || aabbMax.x < rayStart.x) return false;
    }
    else
    {
        denom = 1.f / rayDir.x;
        t1 = (aabbMin.x - rayStart.x) * denom;
        t2 = (aabbMax.x - rayStart.x) * denom;
        if (t1 > t2)
        {
            auto tmp = t2;
            t2 = t1;
            t1 = tmp;
        }
        t = t1 > t ? t1 : t;
        tmax = t2 < tmax ? t2 : tmax;
        if (t > tmax) return false;
    }
    // checking for y axis
    if (rayDir.y == 0.f)
    {
        if (aabbMin.y > rayStart.y || aabbMax.y < rayStart.y) return false;
    }
    else
    {
        denom = 1.f / rayDir.y;
        t1 = (aabbMin.y - rayStart.y) * denom;
        t2 = (aabbMax.y - rayStart.y) * denom;
        if (t1 > t2)
        {
            auto tmp = t2;
            t2 = t1;
            t1 = tmp;
        }
        t = t1 > t ? t1 : t;
        tmax = t2 < tmax ? t2 : tmax;
        if (t > tmax) return false;
    }
    // checking for z axis
    if (rayDir.z == 0.f)
    {
        if (aabbMin.z > rayStart.z || aabbMax.z < rayStart.z) return false;
    }
    else
    {
        denom = 1.f / rayDir.z;
        t1 = (aabbMin.z - rayStart.z) * denom;
        t2 = (aabbMax.z - rayStart.z) * denom;
        if (t1 > t2)
        {
            auto tmp = t2;
            t2 = t1;
            t1 = tmp;
        }
        t = t1 > t ? t1 : t;
        tmax = t2 < tmax ? t2 : tmax;
        if (t > tmax) return false;
    }

    return true;
}
