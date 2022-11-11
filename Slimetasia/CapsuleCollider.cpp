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
    auto currentLayerID = Renderer::Instance().GetCurrentEditorLayer()->GetId();
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

        pts.emplace_back(mypos.x + a.x, mypos.y + d.z, mypos.z + a.y);
        pts.emplace_back(mypos.x + b.x, mypos.y + d.z, mypos.z + b.y);
        pts.emplace_back(mypos.x + a.x, mypos.y - d.z, mypos.z + a.y);
        pts.emplace_back(mypos.x + b.x, mypos.y - d.z, mypos.z + b.y);

        if (i % 90 == 0)
        {
            pts.emplace_back(mypos.x + a.x, mypos.y + d.z, mypos.z + a.y);
            pts.emplace_back(mypos.x + a.x, mypos.y - d.z, mypos.z + a.y);
        }

        Vector3 dud = i < 180 ? d : -d;

        pts.emplace_back(mypos.x, mypos.y + a.x + dud.z, mypos.z + a.y);
        pts.emplace_back(mypos.x, mypos.y + b.x + dud.z, mypos.z + b.y);
        pts.emplace_back(mypos.x + a.y, mypos.y + a.x + dud.z, mypos.z);
        pts.emplace_back(mypos.x + b.y, mypos.y + b.x + dud.z, mypos.z);
    }

    Renderer::Instance().DrawDebug(currentLayerID, pts, Color4(0.0f, 1.0f, 0.0f, 1.0f), DebugPrimitiveType::Lines);
}

bool CapsuleCollider::ContainsPoint(const Vector3& pt)
{
    const float diffYCenterSphere1 = pt.y - m_HalfHeight;
    const float diffYCenterSphere2 = pt.y + m_HalfHeight;
    const float xSquare = pt.x * pt.x;
    const float zSquare = pt.z * pt.z;
    const float squareRadius = m_radius * m_radius;

    // Return true if the point is inside the cylinder or one of the two spheres of the capsule
    return xSquare + zSquare < squareRadius && pt.y < m_HalfHeight && pt.y > -m_HalfHeight || xSquare + zSquare + diffYCenterSphere1 * diffYCenterSphere1 < squareRadius ||
           xSquare + zSquare + diffYCenterSphere2 * diffYCenterSphere2 < squareRadius;
}

bool CapsuleCollider::Raycast(const Ray& ray, RaycastData_tmp& data)
{
    return false;
}

void CapsuleCollider::ComputeInertiaTensor(Matrix3& tensor, const float& mass) const
{
    tensor.SetAllValues(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
}

Vector3 CapsuleCollider::GetClosestPoint(const Vector3& inp) const
{
    auto pos = GetPosition();
    Vector3 top { pos.x, pos.y + m_HalfHeight, pos.z };
    Vector3 bot { pos.x, pos.y - m_HalfHeight, pos.z };

    Vector3 dist = inp - bot;
    Vector3 normal = (top - bot).Normalized();
    Vector3 result = dist.Projection(normal);

    result += bot;

    result.x = result.x > top.x ? top.x : result.x < bot.x ? bot.x : result.x;
    result.y = result.y > top.y ? top.y : result.y < bot.y ? bot.y : result.y;
    result.z = result.z > top.z ? top.z : result.z < bot.z ? bot.z : result.z;

    return result;
}

REFLECT_INIT(CapsuleCollider)
REFLECT_PARENT(CollisionMesh_3D)
REFLECT_PROPERTY(m_offset)
REFLECT_PROPERTY(m_HalfHeight)
REFLECT_PROPERTY(m_radius)
REFLECT_END()
