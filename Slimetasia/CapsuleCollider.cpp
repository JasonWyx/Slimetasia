#include "CapsuleCollider.h"

#include "Ray.h"
#include "RaycastInfo.h"
#include "Renderer.h"

CapsuleCollider::CapsuleCollider(GameObject* parentObject, const float& radius, const float& halfHeight)
    : CollisionMesh_3D { parentObject, "CapsuleCollider", eCollisionShapeType_CAPSULE, eCollisionShape_CAPSULE, radius }
    , m_HalfHeight { halfHeight }
{
}

void CapsuleCollider::DebugDraw()
{
    auto parentLayer = GetOwner()->GetParentLayer();

    if (parentLayer == nullptr) return;

    auto pid = parentLayer->GetId();
#ifdef USE_VULKAN
    auto currentLayerID = 0U; // todo
#else
    auto currentLayerID = Renderer::Instance().GetCurrentEditorLayer()->GetId();
#endif  // USE_VULKAN
    if (pid != currentLayerID) return;
    std::vector<Vector3> pts;
    auto u = Vector3 { 1.f, 0.f, 0.f }, v = Vector3 { 0.f, 1.f, 0.f }, w = Vector3 { 0.f, 0.f, 1.f };
    const auto mypos = GetPosition();

    Vector3 d(0, 0, m_HalfHeight);

    for (int i = 0; i < 360; i++)
    {
        float ra = Math::ToRadians((float)i);
        float rb = Math::ToRadians((float)i + 1);
        Vector2 a = Vector2(sinf(ra), cosf(ra)) * m_radius;
        Vector2 b = Vector2(sinf(rb), cosf(rb)) * m_radius;

        pts.emplace_back(mypos[0] + a[0], mypos[1] + d[2], mypos[2] + a[1]);
        pts.emplace_back(mypos[0] + b[0], mypos[1] + d[2], mypos[2] + b[1]);
        pts.emplace_back(mypos[0] + a[0], mypos[1] - d[2], mypos[2] + a[1]);
        pts.emplace_back(mypos[0] + b[0], mypos[1] - d[2], mypos[2] + b[1]);

        if (i % 90 == 0)
        {
            pts.emplace_back(mypos[0] + a[0], mypos[1] + d[2], mypos[2] + a[1]);
            pts.emplace_back(mypos[0] + a[0], mypos[1] - d[2], mypos[2] + a[1]);
        }

        Vector3 dud = i < 180 ? d : -d;

        pts.emplace_back(mypos[0], mypos[1] + a[0] + dud[2], mypos[2] + a[1]);
        pts.emplace_back(mypos[0], mypos[1] + b[0] + dud[2], mypos[2] + b[1]);
        pts.emplace_back(mypos[0] + a[1], mypos[1] + a[0] + dud[2], mypos[2]);
        pts.emplace_back(mypos[0] + b[1], mypos[1] + b[0] + dud[2], mypos[2]);
    }

    #ifdef USE_VULKAN
#else
    Renderer::Instance().DrawDebug(currentLayerID, pts, Color4(0.0f, 1.0f, 0.0f, 1.0f), DebugPrimitiveType::Lines);
#endif  // USE_VULKAN
}

bool CapsuleCollider::ContainsPoint(const Vector3& pt)
{
    const float diffYCenterSphere1 = pt[1] - m_HalfHeight;
    const float diffYCenterSphere2 = pt[1] + m_HalfHeight;
    const float xSquare = pt[0] * pt[0];
    const float zSquare = pt[2] * pt[2];
    const float squareRadius = m_radius * m_radius;

    // Return true if the point is inside the cylinder or one of the two spheres of the capsule
    return xSquare + zSquare < squareRadius && pt[1] < m_HalfHeight && pt[1] > -m_HalfHeight || xSquare + zSquare + diffYCenterSphere1 * diffYCenterSphere1 < squareRadius ||
           xSquare + zSquare + diffYCenterSphere2 * diffYCenterSphere2 < squareRadius;
}

bool CapsuleCollider::Raycast(const Ray& ray, RaycastData_tmp& data)
{
    return false;
}

void CapsuleCollider::ComputeInertiaTensor(Matrix3& tensor, const float& mass) const
{
    tensor.Fill(0.f);
}

Vector3 CapsuleCollider::GetClosestPoint(const Vector3& inp) const
{
    auto pos = GetPosition();
    Vector3 top { pos[0], pos[1] + m_HalfHeight, pos[2] };
    Vector3 bot { pos[0], pos[1] - m_HalfHeight, pos[2] };

    Vector3 dist = inp - bot;
    Vector3 normal = (top - bot).Normalized();
    Vector3 result = dist.Projection(normal);

    result += bot;

    result[0] = result[0] > top[0] ? top[0] : result[0] < bot[0] ? bot[0] : result[0];
    result[1] = result[1] > top[1] ? top[1] : result[1] < bot[1] ? bot[1] : result[1];
    result[2] = result[2] > top[2] ? top[2] : result[2] < bot[2] ? bot[2] : result[2];

    return result;
}

REFLECT_INIT(CapsuleCollider)
REFLECT_PARENT(CollisionMesh_3D)
REFLECT_PROPERTY(m_offset)
REFLECT_PROPERTY(m_HalfHeight)
REFLECT_PROPERTY(m_radius)
REFLECT_END()
