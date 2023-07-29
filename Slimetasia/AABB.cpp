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
    m_Min[0] = m_Min[1] = m_Min[2] = MAX_FLOAT;
    m_Max[0] = m_Max[1] = m_Max[2] = MIN_FLOAT;
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
    Vector3 u { m_Max[0] - m_Min[0], 0.f, 0.f };
    Vector3 v { 0.f, m_Max[1] - m_Min[1], 0.f };
    Vector3 w { 0.f, 0.f, m_Max[2] - m_Min[2] };

    return abs(u.Cross(v).Dot(w));
}

float AABB::GetSurfaceArea() const
{
    /******Student:Assignment2******/
    // Return the aabb's surface area
    // Warn("Assignment2: Required function un-implemented");
    Vector3 dimens(m_Max - m_Min);

    return 2.f * (dimens[0] * dimens[1] + dimens[0] * dimens[2] + dimens[1] * dimens[2]);
}

bool AABB::Contains(const AABB& aabb) const
{
    /******Student:Assignment2******/
    // Return if aabb is completely contained in this
    // Warn("Assignment2: Required function un-implemented");
    return (aabb.m_Min[0] <= m_Max[0] && aabb.m_Min[0] >= m_Min[0] && aabb.m_Max[0] <= m_Max[0] && aabb.m_Max[0] >= m_Min[0] && aabb.m_Min[1] <= m_Max[1] && aabb.m_Min[1] >= m_Min[1] && aabb.m_Max[1] <= m_Max[1] &&
        aabb.m_Max[1] >= m_Min[1] && aabb.m_Min[2] <= m_Max[2] && aabb.m_Min[2] >= m_Min[2] && aabb.m_Max[2] <= m_Max[2] && aabb.m_Max[2] >= m_Min[2]);
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
    const Vector3 newextent(abs(rotation[0]) * extent[0] + abs(rotation[1]) * extent[1] + abs(rotation[2]) * extent[2],
        abs(rotation[3]) * extent[0] + abs(rotation[4]) * extent[1] + abs(rotation[5]) * extent[2], abs(rotation[6]) * extent[0] + abs(rotation[7]) * extent[1] + abs(rotation[8]) * extent[2]);

    center *= scale;
    Vector3 newcenter(rotation[0] * center[0] + rotation[1] * center[1] + rotation[2] * center[2], rotation[3] * center[0] + rotation[4] * center[1] + rotation[5] * center[2],
        rotation[6] * center[0] + rotation[7] * center[1] + rotation[8] * center[2]);

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

#ifdef USE_VULKAN
    auto currentLayerID = 0U; // todo
#else
    auto currentLayerID = Renderer::Instance().GetCurrentEditorLayer()->GetId();
#endif  // USE_VULKAN
    if (parentID != currentLayerID) return;

    std::vector<Vector3> pts;

    pts.emplace_back(m_Max);
    pts.emplace_back(m_Max[0], m_Max[1], m_Min[2]);

    pts.emplace_back(m_Max);
    pts.emplace_back(m_Max[0], m_Min[1], m_Max[2]);

    pts.emplace_back(m_Max);
    pts.emplace_back(m_Min[0], m_Max[1], m_Max[2]);

    pts.emplace_back(m_Min[0], m_Min[1], m_Max[2]);
    pts.emplace_back(m_Max[0], m_Min[1], m_Max[2]);

    pts.emplace_back(m_Max[0], m_Min[1], m_Min[2]);
    pts.emplace_back(m_Max[0], m_Min[1], m_Max[2]);

    pts.emplace_back(m_Max[0], m_Max[1], m_Min[2]);
    pts.emplace_back(m_Max[0], m_Min[1], m_Min[2]);

    pts.emplace_back(m_Min);
    pts.emplace_back(m_Max[0], m_Min[1], m_Min[2]);

    pts.emplace_back(m_Min);
    pts.emplace_back(m_Min[0], m_Min[1], m_Max[2]);

    pts.emplace_back(m_Min);
    pts.emplace_back(m_Min[0], m_Max[1], m_Min[2]);

    pts.emplace_back(m_Min[0], m_Max[1], m_Max[2]);
    pts.emplace_back(m_Min[0], m_Min[1], m_Max[2]);

    pts.emplace_back(m_Min[0], m_Max[1], m_Min[2]);
    pts.emplace_back(m_Min[0], m_Max[1], m_Max[2]);

    pts.emplace_back(m_Min[0], m_Max[1], m_Min[2]);
    pts.emplace_back(m_Max[0], m_Max[1], m_Min[2]);

#ifdef USE_VULKAN
#else
    Renderer::Instance().DrawDebug(currentLayerID, pts, Color4(0.0f, 1.0f, 1.0f, 1.0f), DebugPrimitiveType::Lines);
#endif  // USE_VULKAN

#endif
}

bool AABB::AABBAABB(const Vector3& aabbMin0, const Vector3& aabbMax0, const Vector3& aabbMin1, const Vector3& aabbMax1)
{
    return !((aabbMin1[0] > aabbMax0[0] || aabbMin0[0] > aabbMax1[0]) || (aabbMin1[1] > aabbMax0[1] || aabbMin0[1] > aabbMax1[1]) || (aabbMin1[2] > aabbMax0[2] || aabbMin0[2] > aabbMax1[2]));
}

bool AABB::RayAabb(const Vector3& rayStart, const Vector3& rayDir, const Vector3& aabbMin, const Vector3& aabbMax, float& t)
{
    Vector4 plane_l { 1.f, 0.f, 0.f, aabbMin[0] }, plane_bo { 0.f, 1.f, 0.f, aabbMin[1] }, plane_f { 0.f, 0.f, 1.f, aabbMin[2] };
    auto denom = 0.f;
    t = 0.f;
    auto tmax = FLT_MAX, t1 = 0.f, t2 = 0.f;
    // checking for x axis
    if (rayDir[0] == 0.f)
    {
        if (aabbMin[0] > rayStart[0] || aabbMax[0] < rayStart[0]) return false;
    }
    else
    {
        denom = 1.f / rayDir[0];
        t1 = (aabbMin[0] - rayStart[0]) * denom;
        t2 = (aabbMax[0] - rayStart[0]) * denom;
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
    if (rayDir[1] == 0.f)
    {
        if (aabbMin[1] > rayStart[1] || aabbMax[1] < rayStart[1]) return false;
    }
    else
    {
        denom = 1.f / rayDir[1];
        t1 = (aabbMin[1] - rayStart[1]) * denom;
        t2 = (aabbMax[1] - rayStart[1]) * denom;
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
    if (rayDir[2] == 0.f)
    {
        if (aabbMin[2] > rayStart[2] || aabbMax[2] < rayStart[2]) return false;
    }
    else
    {
        denom = 1.f / rayDir[2];
        t1 = (aabbMin[2] - rayStart[2]) * denom;
        t2 = (aabbMax[2] - rayStart[2]) * denom;
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
