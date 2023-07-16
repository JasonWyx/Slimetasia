#include "BoxCollider.h"

#include "Ray.h"
#include "RaycastInfo.h"
#include "Renderer.h"

BoxCollider::BoxCollider(GameObject* parentObject, const Vector3& halfExtent)
    : ConvexCollider { parentObject, "BoxCollider", eCollisionShapeType_CONVEX_POLY, eCollisionShape_BOX }
    , m_HalfExtent { halfExtent }
{
    assert(halfExtent.x >= 0.f && halfExtent.y >= 0.f && halfExtent.z >= 0.f);

    // box has 8 verts, add all to
    m_EdgeData.AddVertex(0);
    m_EdgeData.AddVertex(1);
    m_EdgeData.AddVertex(2);
    m_EdgeData.AddVertex(3);
    m_EdgeData.AddVertex(4);
    m_EdgeData.AddVertex(5);
    m_EdgeData.AddVertex(6);
    m_EdgeData.AddVertex(7);

    m_EdgeData.AddFace({ 0, 1, 2, 3 });
    m_EdgeData.AddFace({ 1, 5, 6, 2 });
    m_EdgeData.AddFace({ 5, 4, 7, 6 });
    m_EdgeData.AddFace({ 4, 0, 3, 7 });
    m_EdgeData.AddFace({ 4, 5, 1, 0 });
    m_EdgeData.AddFace({ 0, 1, 2, 3 });

    m_EdgeData.Initialize();
}

BoxCollider::BoxCollider(GameObject* parentObject, const float& width, const float& height, const float& depth)
    : ConvexCollider { parentObject, "BoxCollider", eCollisionShapeType_CONVEX_POLY, eCollisionShape_BOX }
    , m_HalfWidth { width }
    , m_HalfHeight { height }
    , m_HalfDepth { depth }
{
    assert(width >= 0.f && height >= 0.f && depth >= 0.f);

    // box has 8 verts, add all to
    m_EdgeData.AddVertex(0);
    m_EdgeData.AddVertex(1);
    m_EdgeData.AddVertex(2);
    m_EdgeData.AddVertex(3);
    m_EdgeData.AddVertex(4);
    m_EdgeData.AddVertex(5);
    m_EdgeData.AddVertex(6);
    m_EdgeData.AddVertex(7);

    m_EdgeData.AddFace({ 0, 1, 2, 3 });
    m_EdgeData.AddFace({ 1, 5, 6, 2 });
    m_EdgeData.AddFace({ 5, 4, 7, 6 });
    m_EdgeData.AddFace({ 4, 0, 3, 7 });
    m_EdgeData.AddFace({ 4, 5, 1, 0 });
    m_EdgeData.AddFace({ 0, 1, 2, 3 });

    m_EdgeData.Initialize();
}

Vector3 BoxCollider::GetVertexPosition(const uint& index) const
{
    switch (index)
    {
        case 0: return Vector3 { -m_HalfExtent.x, m_HalfExtent.y, -m_HalfExtent.z };
        case 1: return Vector3 { m_HalfExtent.x, m_HalfExtent.y, -m_HalfExtent.z };
        case 2: return Vector3 { m_HalfExtent.x, -m_HalfExtent.y, -m_HalfExtent.z };
        case 3: return Vector3 { -m_HalfExtent };
        case 4: return Vector3 { -m_HalfExtent.x, m_HalfExtent.y, m_HalfExtent.z };
        case 5: return Vector3 { m_HalfExtent };
        case 6: return Vector3 { m_HalfExtent.x, -m_HalfExtent.y, m_HalfExtent.z };
        case 7: return Vector3 { -m_HalfExtent.x, -m_HalfExtent.y, m_HalfExtent.z };
        default: std::cout << "Index is out of range! BoxCollider::GetVertexPosition!\n"; assert(false);
    }

    return Vector3();
}

EdgeData::Vertex BoxCollider::GetVertex(const uint& index) const
{
    return m_EdgeData.GetVertex(index);
}

EdgeData::Face BoxCollider::GetFace(const uint& index) const
{
    return m_EdgeData.GetFace(index);
}

bool BoxCollider::ContainsPoint(const Vector3& pt)
{
    Vector3 max = GetPosition() + m_HalfExtent;
    Vector3 min = GetPosition() - m_HalfExtent;

    return (pt.x >= min.x && pt.x <= max.x && pt.y >= min.y && pt.y <= max.y && pt.z >= min.z && pt.z <= max.z);
}

void BoxCollider::ComputeBounds(Vector3& min, Vector3& max) const
{
    auto pos = GetPosition() + GetOffset();

    min = pos - m_HalfExtent;

    max = pos + m_HalfExtent;
}

void BoxCollider::Initialize()
{
    m_EdgeData.ClearData();

    // box has 8 verts, add all to
    m_EdgeData.AddVertex(0);
    m_EdgeData.AddVertex(1);
    m_EdgeData.AddVertex(2);
    m_EdgeData.AddVertex(3);
    m_EdgeData.AddVertex(4);
    m_EdgeData.AddVertex(5);
    m_EdgeData.AddVertex(6);
    m_EdgeData.AddVertex(7);

    m_EdgeData.AddFace({ 0, 1, 2, 3 });
    m_EdgeData.AddFace({ 1, 5, 6, 2 });
    m_EdgeData.AddFace({ 5, 4, 7, 6 });
    m_EdgeData.AddFace({ 4, 0, 3, 7 });
    m_EdgeData.AddFace({ 4, 5, 1, 0 });
    m_EdgeData.AddFace({ 0, 1, 2, 3 });

    m_EdgeData.Initialize();
}

void BoxCollider::DebugDraw()
{
    auto pid = GetOwner()->GetParentLayer()->GetId();
#ifdef USE_VULKAN
    auto currentLayerID = 0U;
#else
    auto currentLayerID = Renderer::Instance().GetCurrentEditorLayer()->GetId();
#endif  // USE_VULKAN
    if (pid != currentLayerID) return;

    std::vector<Vector3> pts;
    Vector3 position = GetPosition() + m_offset;

    Vector3 minPt(position.x - m_HalfExtent.x, position.y - m_HalfExtent.y, position.z - m_HalfExtent.z);
    Vector3 maxPt(position.x + m_HalfExtent.x, position.y + m_HalfExtent.y, position.z + m_HalfExtent.z);

    pts.emplace_back(maxPt);
    pts.emplace_back(Vector3 { maxPt.x, maxPt.y, minPt.z });

    pts.emplace_back(maxPt);
    pts.emplace_back(Vector3 { maxPt.x, minPt.y, maxPt.z });

    pts.emplace_back(maxPt);
    pts.emplace_back(Vector3 { minPt.x, maxPt.y, maxPt.z });

    pts.emplace_back(Vector3 { minPt.x, minPt.y, maxPt.z });
    pts.emplace_back(Vector3 { maxPt.x, minPt.y, maxPt.z });

    pts.emplace_back(Vector3 { maxPt.x, minPt.y, minPt.z });
    pts.emplace_back(Vector3 { maxPt.x, minPt.y, maxPt.z });

    pts.emplace_back(Vector3 { maxPt.x, maxPt.y, minPt.z });
    pts.emplace_back(Vector3 { maxPt.x, minPt.y, minPt.z });

    pts.emplace_back(minPt);
    pts.emplace_back(Vector3 { maxPt.x, minPt.y, minPt.z });

    pts.emplace_back(minPt);
    pts.emplace_back(Vector3 { minPt.x, minPt.y, maxPt.z });

    pts.emplace_back(minPt);
    pts.emplace_back(Vector3 { minPt.x, maxPt.y, minPt.z });

    pts.emplace_back(Vector3 { minPt.x, maxPt.y, maxPt.z });
    pts.emplace_back(Vector3 { minPt.x, minPt.y, maxPt.z });

    pts.emplace_back(Vector3 { minPt.x, maxPt.y, minPt.z });
    pts.emplace_back(Vector3 { minPt.x, maxPt.y, maxPt.z });

    pts.emplace_back(Vector3 { minPt.x, maxPt.y, minPt.z });
    pts.emplace_back(Vector3 { maxPt.x, maxPt.y, minPt.z });

#ifdef USE_VULKAN
#else
    Renderer::Instance().DrawDebug(currentLayerID, pts, Color4(0.0f, 1.0f, 0.0f, 1.0f), DebugPrimitiveType::Lines);
#endif  // USE_VULKAN
}

void BoxCollider::ComputeInertiaTensor(Matrix3& tensor, const float& mass) const
{
    // float factor = 1.f / 3.f * mass;
    // float xSq = m_HalfExtent.x * m_HalfExtent.x;
    // float ySq = m_HalfExtent.y * m_HalfExtent.y;
    // float zSq = m_HalfExtent.z * m_HalfExtent.z;
    //
    // tensor = Matrix3(factor * (ySq + zSq), 0.f, 0.f,
    //				 0.f, factor * (xSq + zSq), 0.f,
    //				 0.f, 0.f, factor * (xSq + ySq));

    tensor = Matrix3(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
}

// TODO : Shift this function into the parent of box collider : ConvexPolygon collider!
// this function returns the support point with the position added to it!
Vector3 BoxCollider::GetSupportPoint(const Vector3& dir) const
{
    float largestDist = FLT_MIN;
    Vector3 chosenVertex;

    Vector3 currPos = GetPosition();

    // iterate through the entire range of verts
    for (uint i = 0; i < 8; i++)
    {
        Vector3 currVert = currPos + GetVertexPosition(i);

        float proj = currVert.Dot(dir);

        if (proj > largestDist)
        {
            largestDist = proj;
            chosenVertex = currVert;
        }
    }

    return chosenVertex;
}

Vector3 BoxCollider::GetPointOnEdge(const Vector3& pt) const
{
    // find out which is the closest face to the pt.
    Vector3 pos = GetPosition();

    // return an arbitrary point on the edge if the point is on the centroid.
    if (pos == pt) return Vector3 { pos.x + m_HalfWidth, pos.y, pos.z };

    return Vector3();
}

bool BoxCollider::Raycast(const Ray& ray, RaycastData_tmp& data)
{
    Vector3 pos = GetPosition() + GetOffset();
    Vector3 maxPt(pos + m_HalfExtent);
    Vector3 minPt(pos - m_HalfExtent);
    float t = 0.f;
    Vector4 plane_l { 1.f, 0.f, 0.f, minPt.x }, plane_bo { 0.f, 1.f, 0.f, minPt.y }, plane_f { 0.f, 0.f, 1.f, minPt.z };
    auto denom = 0.f;
    t = 0.f;
    auto tmax = FLT_MAX, t1 = 0.f, t2 = 0.f;
    // checking for x axis
    if (ray.m_dir.x == 0.f)
    {
        if (minPt.x > ray.m_start.x || maxPt.x < ray.m_start.x) return false;
    }
    else
    {
        denom = 1.f / ray.m_dir.x;
        t1 = (minPt.x - ray.m_start.x) * denom;
        t2 = (maxPt.x - ray.m_start.x) * denom;
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
    if (ray.m_dir.y == 0.f)
    {
        if (minPt.y > ray.m_start.y || maxPt.y < ray.m_start.y) return false;
    }
    else
    {
        denom = 1.f / ray.m_dir.y;
        t1 = (minPt.y - ray.m_start.y) * denom;
        t2 = (maxPt.y - ray.m_start.y) * denom;
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
    if (ray.m_dir.z == 0.f)
    {
        if (minPt.z > ray.m_start.z || maxPt.z < ray.m_start.z) return false;
    }
    else
    {
        denom = 1.f / ray.m_dir.z;
        t1 = (minPt.z - ray.m_start.z) * denom;
        t2 = (maxPt.z - ray.m_start.z) * denom;
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

    data.m_HitFrac = t;
    data.m_WorldHitPt = ray.m_start + t * ray.m_dir;
    data.m_WorldNormal = (GetPosition() - ray.m_start).Normalized();
    data.m_HitObject = m_OwnerObject;

    return true;
}

REFLECT_INIT(BoxCollider)
REFLECT_PARENT(ConvexCollider)
REFLECT_PROPERTY(m_offset)
REFLECT_PROPERTY(m_HalfWidth)
REFLECT_PROPERTY(m_HalfHeight)
REFLECT_PROPERTY(m_HalfDepth)
REFLECT_END()
