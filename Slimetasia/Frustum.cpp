#include "Frustum.h"

#include "Layer.h"
#include "Renderer.h"

Vector4* Frustum::GetPlanes() const
{
    return reinterpret_cast<Vector4*>(const_cast<Plane*>(m_Planes));
}

void Frustum::Set(const Vector3& lbn, const Vector3& rbn, const Vector3& rtn, const Vector3& ltn, const Vector3& lbf, const Vector3& rbf, const Vector3& rtf, const Vector3& ltf)
{
    m_Vertices[0] = lbn;
    m_Vertices[1] = rbn;
    m_Vertices[2] = rtn;
    m_Vertices[3] = ltn;
    m_Vertices[4] = lbf;
    m_Vertices[5] = rbf;
    m_Vertices[6] = rtf;
    m_Vertices[7] = ltf;

    // left
    m_Planes[0].Set(lbf, ltf, lbn);
    // right
    m_Planes[1].Set(rbn, rtf, rbf);
    // top
    m_Planes[2].Set(ltn, ltf, rtn);
    // bot
    m_Planes[3].Set(rbn, lbf, lbn);
    // near
    m_Planes[4].Set(lbn, ltn, rbn);
    // far
    m_Planes[5].Set(rbf, rtf, lbf);
}

void Frustum::DebugDraw(const unsigned int& parentID)
{
#ifdef USE_VULKAN
    auto currentLayerID = 0U; // todo
#else
    auto currentLayerID = Renderer::Instance().GetCurrentEditorLayer()->GetId();
#endif  // USE_VULKAN
    if (parentID != currentLayerID) return;

    std::vector<Vector3> pts;

    pts.emplace_back(m_Vertices[0]);
    pts.emplace_back(m_Vertices[1]);

    pts.emplace_back(m_Vertices[0]);
    pts.emplace_back(m_Vertices[3]);

    pts.emplace_back(m_Vertices[0]);
    pts.emplace_back(m_Vertices[4]);

    pts.emplace_back(m_Vertices[6]);
    pts.emplace_back(m_Vertices[7]);

    pts.emplace_back(m_Vertices[6]);
    pts.emplace_back(m_Vertices[5]);

    pts.emplace_back(m_Vertices[6]);
    pts.emplace_back(m_Vertices[2]);

    pts.emplace_back(m_Vertices[1]);
    pts.emplace_back(m_Vertices[5]);

    pts.emplace_back(m_Vertices[1]);
    pts.emplace_back(m_Vertices[2]);

    pts.emplace_back(m_Vertices[4]);
    pts.emplace_back(m_Vertices[5]);

    pts.emplace_back(m_Vertices[4]);
    pts.emplace_back(m_Vertices[7]);

    pts.emplace_back(m_Vertices[3]);
    pts.emplace_back(m_Vertices[2]);

    pts.emplace_back(m_Vertices[3]);
    pts.emplace_back(m_Vertices[7]);

    #ifdef USE_VULKAN
#else
    Renderer::Instance().DrawDebug(currentLayerID, pts, Color4(1.0f, 0.0f, 1.0f, 1.0f), DebugPrimitiveType::Lines);
#endif  // USE_VULKAN
}

IntersectionType::Type Frustum::FrustumAabb(const Vector4 planes[6], const Vector3& aabbMin, const Vector3& aabbMax, size_t& lastAxis)
{
    IntersectionType::Type type { IntersectionType::NotImplemented };
    bool overlap = false;
    for (auto i = 0u; i < 6u; ++i)
    {
        auto j = (i + lastAxis) % 6;
        type = Plane::PlaneAabb(planes[j], aabbMin, aabbMax);
        if (type == IntersectionType::Outside)
        {
            lastAxis = j;
            return type;
        }
        overlap = type == IntersectionType::Overlaps ? true : overlap;
    }
    return overlap ? IntersectionType::Overlaps : IntersectionType::Inside;
}
