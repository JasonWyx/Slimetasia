#include "SphereCollider.h"

#include "Ray.h"
#include "RaycastInfo.h"
#include "Renderer.h"

SphereCollider::SphereCollider(GameObject* parentObject, const float& radius)
    : CollisionMesh_3D{parentObject, "SphereCollider", eCollisionShapeType_SPHERE, eCollisionShape_SPHERE, radius}
{
}

void SphereCollider::DebugDraw()
{
    auto parent = GetOwner();
    if (!parent) return;

    auto pid = parent->GetParentLayer()->GetId();
    auto currentLayerID = Renderer::Instance().GetCurrentEditorLayer()->GetId();
    if (pid != currentLayerID) return;
    std::vector<Vector3> pts;
    auto u = Vector3{1.f, 0.f, 0.f}, v = Vector3{0.f, 1.f, 0.f}, w = Vector3{0.f, 0.f, 1.f};
    const auto mypos = GetPosition() + m_offset;
    // drawing the disc.
    for (auto i = 0u, j = 0u; i < discpoints; ++i)
    {
        j = i + 1u == discpoints ? 0u : i + 1u;

        pts.emplace_back(mypos + m_radius * (v * cosf(j * DITHER) + w * sinf(j * DITHER)));
        pts.emplace_back(mypos + m_radius * (v * cosf(i * DITHER) + w * sinf(i * DITHER)));

        pts.emplace_back(mypos + m_radius * (u * cosf(j * DITHER) + w * sinf(j * DITHER)));
        pts.emplace_back(mypos + m_radius * (u * cosf(i * DITHER) + w * sinf(i * DITHER)));

        pts.emplace_back(mypos + m_radius * (u * cosf(j * DITHER) + v * sinf(j * DITHER)));
        pts.emplace_back(mypos + m_radius * (u * cosf(i * DITHER) + v * sinf(i * DITHER)));
    }
    auto cam = const_cast<Layer*>(Renderer::Instance().GetCurrentEditorLayer())->GetEditorCamera();
    if (cam)
    {
        const auto eye = cam->GetTransform()->GetWorldPosition();
        const auto eyevec = mypos - eye;
        const auto d = eyevec.Length();
        const auto l = sqrtf(d * d - (m_radius * m_radius));
        const auto newrad = (m_radius / d) * l;
        const auto z = (m_radius / d) * sqrt(d * d - (l * l));
        const auto newcenter = mypos + z * ((eye - mypos) / d);

        auto new_v = w.Cross(eyevec);
        new_v = new_v.Normalized();
        auto new_w = eyevec.Cross(new_v);
        new_w = new_w.Normalized();

        for (auto i = 0u, j = 0u; i < discpoints; ++i)
        {
            j = i + 1u >= discpoints ? 0u : i + 1u;
            pts.emplace_back(newcenter + newrad * (new_v * cosf(j * DITHER) + new_w * sinf(j * DITHER)));
            pts.emplace_back(newcenter + newrad * (new_v * cosf(i * DITHER) + new_w * sinf(i * DITHER)));
        }
    }

    Renderer::Instance().DrawDebug(currentLayerID, pts, Color4(0.0f, 1.0f, 0.0f, 1.0f), DebugPrimitiveType::Lines);
}

bool SphereCollider::ContainsPoint(const Vector3& pt)
{
    Vector3 dist = pt - GetPosition() + m_offset;

    return (dist.SquareLength() < m_radius * m_radius);
}

bool SphereCollider::Raycast(const Ray& ray, RaycastData_tmp& data)
{
    if (ContainsPoint(ray.m_start)) return false;

    Vector3 m = GetPosition() + GetOffset() - ray.m_start;
    if (m.SquareLength() < m_radius * m_radius)
    {
        data.m_HitFrac = 0.f;
        data.m_WorldHitPt = ray.m_start;
        data.m_WorldNormal = Vector3(0.f, 1.f, 0.f);
        data.m_HitObject = m_OwnerObject;
        return true;
    }

    float t = 0.f;
    auto a = ray.m_dir.Dot(ray.m_dir);
    auto b = m.Dot(ray.m_dir) * -2.f;
    auto c = m.Dot(m) - m_radius * m_radius;
    auto tmp = b * b - 4.f * a * c;
    if (tmp < 0.f) return false;
    if (!tmp)
        t = (-b + sqrtf(tmp)) / (2.f * a);
    else
    {
        auto t1 = (-b + sqrtf(tmp)) / (2.f * a);
        auto t2 = (-b - sqrtf(tmp)) / (2.f * a);
        t = t1 < t2 && t1 >= 0.f ? t1 : t2;
    }

    if (t > 0.f)
    {
        data.m_HitFrac = t;
        data.m_WorldHitPt = ray.m_start + t * ray.m_dir;
        data.m_WorldNormal = m.Normalized();
        data.m_HitObject = m_OwnerObject;
        return true;
    }

    return false;
}

void SphereCollider::ComputeInertiaTensor(Matrix3& tensor, const float& mass) const
{
    float diag = 0.4f * mass * m_radius * m_radius;

    tensor = Matrix3(diag, 0.f, 0.f, 0.f, diag, 0.f, 0.f, 0.f, diag);
}

void SphereCollider::ComputeBounds(Vector3& min, Vector3& max) const
{
    auto pos = GetPosition() + GetOffset();

    // auto len = sqrtf(m_radius * m_radius * 2.f);

    min = Vector3(pos.x - m_radius, pos.y - m_radius, pos.z - m_radius);
    max = Vector3(pos.x + m_radius, pos.y + m_radius, pos.z + m_radius);
}

REFLECT_INIT(SphereCollider)
REFLECT_PARENT(CollisionMesh_3D)
REFLECT_END()