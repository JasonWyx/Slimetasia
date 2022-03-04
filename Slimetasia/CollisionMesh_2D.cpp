#include "CollisionMesh_2D.h"

#include "IntersectionData.h"

void Edge_2D::ComputeNormal()
{
    auto dist = m_edge_vector;
    dist.Normalize();
    m_normal.x = -dist.y;
    m_normal.y = dist.x;
}

inline TVector2<float> Edge_2D::GetEdgeVector() const
{
    return m_edge_vector;
}

/*void CollisionMesh_2D::Unregister()
{
    if(m_Registered)
        GetInstance(PhysicsSystem).Deregister2DCollider(this);
}*/

IntersectionData CollisionMesh_2D::IsColliding(CollisionMesh_2D*& othermesh)
{
    switch (othermesh->m_mesh_type)
    {
        case CIRCLE:
            switch (m_mesh_type)
            {
                case CIRCLE: return IsCollidingCirclevsCircle(othermesh); break;
                case AABB_COL_MESH: return IsCollidingCirclevsAABB(othermesh); break;
                case POLYGON: return IsCollidingCirclevsPolygon(othermesh); break;
                default:
                    std::cout << "The CollisionMesh's m_mesh_type is invalid." << std::endl;
                    assert(0);
                    return IntersectionData{};
            }
            break;
        case AABB_COL_MESH:
            switch (m_mesh_type)
            {
                case CIRCLE: return IsCollidingAABBvsCircle(othermesh); break;
                case AABB_COL_MESH: return IsCollidingAABBvsAABB(othermesh); break;
                case POLYGON: return IsCollidingAABBvsPolygon(othermesh); break;
                default:
                    std::cout << "The CollisionMesh's m_mesh_type is invalid." << std::endl;
                    assert(0);
                    return IntersectionData{};
            }
            break;
        case POLYGON:
            switch (m_mesh_type)
            {
                case CIRCLE: return IsCollidingPolygonvsCircle(othermesh); break;
                case AABB_COL_MESH: return IsCollidingPolygonvsAABB(othermesh); break;
                case POLYGON: return IsCollidingPolygonvsPolygon(othermesh); break;
                default:
                    std::cout << "The CollisionMesh's m_mesh_type is invalid." << std::endl;
                    assert(0);
                    return IntersectionData{};
            }
            break;
        default:
            std::cout << "The othermesh parameter in CollisionMesh2D_IsColliding is invalid." << std::endl;
            assert(0);
            return IntersectionData{};
    }
}

IntersectionData CollisionMesh_2D::IsCollidingAABBvsAABB(CollisionMesh_2D*& othermesh)
{
    const auto first = dynamic_cast<AABBColliderMesh*>(this);
    const auto second = dynamic_cast<AABBColliderMesh*>(othermesh);
    const auto firstpos = GetPosition() + GetOffset();
    const auto secondpos = second->GetPosition() + second->GetOffset();
    auto firstmax = firstpos, firstmin = firstpos;
    auto secondmax = secondpos, secondmin = secondpos;

    firstmax.x += first->GetWidth() / 2.f;
    firstmax.y += first->GetHeight() / 2.f;
    firstmin.x -= first->GetWidth() / 2.f;
    firstmin.y -= first->GetHeight() / 2.f;

    secondmax.x += second->GetWidth() / 2.f;
    secondmax.y += second->GetHeight() / 2.f;
    secondmin.x -= second->GetWidth() / 2.f;
    secondmin.y -= second->GetHeight() / 2.f;

    return IntersectionData{((firstmax.x >= secondmin.x && firstmax.x <= secondmax.x && firstmax.y >= secondmin.y && firstmax.y <= secondmax.y) ||
                             (firstmin.x >= secondmin.x && firstmin.x <= secondmax.x && firstmin.y >= secondmin.y && firstmin.y <= secondmax.y)),
                            0.f, firstpos - secondpos};
}

IntersectionData CollisionMesh_2D::IsCollidingAABBvsCircle(CollisionMesh_2D*& othermesh)
{
    const auto first = dynamic_cast<AABBColliderMesh*>(this);
    const auto second = dynamic_cast<CircleColliderMesh*>(othermesh);
    const auto firstpos = GetPosition() + GetOffset();
    const auto secondpos = second->GetPosition() + second->GetOffset();
    auto firstmax = firstpos, firstmin = firstpos;

    firstmax.x += first->GetWidth() / 2.f;
    firstmax.y += first->GetHeight() / 2.f;
    firstmin.x -= first->GetWidth() / 2.f;
    firstmin.y -= first->GetHeight() / 2.f;

    const auto nearestpt = second->GetClosestPointInSphere(firstpos);

    return IntersectionData{nearestpt.x >= firstmin.x && nearestpt.x <= firstmax.x && nearestpt.y >= firstmin.y && nearestpt.y <= firstmax.y, 0.f, firstpos - secondpos};
}

IntersectionData CollisionMesh_2D::IsCollidingAABBvsPolygon(CollisionMesh_2D*& othermesh)
{
    // to be implemented.
    othermesh;
    return IntersectionData{};
}

IntersectionData CollisionMesh_2D::IsCollidingCirclevsAABB(CollisionMesh_2D*& othermesh)
{
    const auto first = dynamic_cast<CircleColliderMesh*>(this);
    const auto second = dynamic_cast<AABBColliderMesh*>(othermesh);
    const auto firstpos = GetPosition() + GetOffset();
    const auto secondpos = second->GetPosition() + second->GetOffset();
    auto secondmax = secondpos, secondmin = secondpos;

    secondmax.x += second->GetWidth() / 2.f;
    secondmax.y += second->GetHeight() / 2.f;
    secondmin.x -= second->GetWidth() / 2.f;
    secondmin.y -= second->GetHeight() / 2.f;

    const auto nearestpt = first->GetClosestPointInSphere(secondpos);

    return IntersectionData{nearestpt.x >= secondmin.x && nearestpt.x <= secondmax.x && nearestpt.y >= secondmin.y && nearestpt.y <= secondmax.y, 0.f, firstpos - secondpos};
}

IntersectionData CollisionMesh_2D::IsCollidingCirclevsCircle(CollisionMesh_2D*& othermesh)
{
    const auto first = dynamic_cast<CircleColliderMesh*>(this);
    const auto second = dynamic_cast<CircleColliderMesh*>(othermesh);
    const auto firstpos = GetPosition() + GetOffset();
    const auto secondpos = second->GetPosition() + second->GetOffset();

    const auto dist = (firstpos - secondpos).SquareLength();
    auto totalrad = first->GetRadius() + second->GetRadius();
    totalrad *= totalrad;

    return IntersectionData{dist <= totalrad, 0.f, firstpos - secondpos};
}

IntersectionData CollisionMesh_2D::IsCollidingCirclevsPolygon(CollisionMesh_2D*& othermesh)
{
    // to be implemented.
    othermesh;
    return IntersectionData{};
}

IntersectionData CollisionMesh_2D::IsCollidingPolygonvsAABB(CollisionMesh_2D*& othermesh)
{
    // to be implemented.
    othermesh;
    return IntersectionData{};
}

IntersectionData CollisionMesh_2D::IsCollidingPolygonvsCircle(CollisionMesh_2D*& othermesh)
{
    // to be implemented.
    othermesh;
    return IntersectionData{};
}

IntersectionData CollisionMesh_2D::IsCollidingPolygonvsPolygon(CollisionMesh_2D*& othermesh)
{
    // to be implemented.
    othermesh;
    return IntersectionData{};
}

bool CollisionMesh_2D::IsCollidingWithMouse(const TVector2<float>& mousepos)
{
    switch (m_mesh_type)
    {
        case CIRCLE: return IsCollidingWithMouseCircle(mousepos); break;

        case AABB_COL_MESH: return IsCollidingWithMouseAABB(mousepos); break;

        case POLYGON: return IsCollidingWithMousePolygon(mousepos); break;

        default: std::cout << "The CollisionMesh's m_mesh_type is invalid." << std::endl; return false;
    }
}

bool CollisionMesh_2D::IsCollidingWithMouseAABB(const TVector2<float>& mousepos)
{
    auto tmp = dynamic_cast<AABBColliderMesh*>(this);
    auto pos = GetPosition() + GetOffset();
    auto max = pos, min = pos;
    max.x += tmp->GetWidth() / 2.f;
    max.y += tmp->GetHeight() / 2.f;
    min.x -= tmp->GetWidth() / 2.f;
    min.y -= tmp->GetHeight() / 2.f;
    return mousepos.x >= min.x && mousepos.x <= max.x && mousepos.y >= min.y && mousepos.y <= max.y;
}

bool CollisionMesh_2D::IsCollidingWithMouseCircle(const TVector2<float>& mousepos)
{
    auto tmp = dynamic_cast<CircleColliderMesh*>(this);
    auto pos = GetPosition() + GetOffset();
    auto diff = TVector2<float>{pos.x - mousepos.x, pos.y - mousepos.y};
    auto radsq = tmp->GetRadius();
    radsq *= radsq;
    auto len = diff.SquareLength();

    return len <= radsq;
}

bool CollisionMesh_2D::IsCollidingWithMousePolygon(const TVector2<float>& mousepos)
{
    // to be implemented.
    return false;
}

Vector2 CircleColliderMesh::GetClosestPointInSphere(const Vector2& pt) const
{
    auto pos = GetPosition();
    return Vector2{pos.x - pt.x, pos.y - pt.y}.Normalize() * m_radius;
}

Vector2 CircleColliderMesh::GetClosestPointInSphere(const Vector3& pt) const
{
    auto pos = GetPosition();
    return Vector2{pos.x - pt.x, pos.y - pt.y}.Normalize() * m_radius;
}

REFLECT_INIT(CollisionMesh_2D)
REFLECT_PARENT(IComponent)
REFLECT_PROPERTY(m_mesh_type)
REFLECT_PROPERTY(m_offset)
REFLECT_END()

REFLECT_INIT(CircleColliderMesh)
REFLECT_PARENT(CollisionMesh_2D)
REFLECT_PROPERTY(m_radius)
REFLECT_END()

REFLECT_INIT(AABBColliderMesh)
REFLECT_PARENT(CollisionMesh_2D)
REFLECT_PROPERTY(m_height)
REFLECT_PROPERTY(m_width)
REFLECT_END()

/*REFLECT_INIT(POLYGONColliderMesh)
REFLECT_PARENT(CollisionMesh_2D)
REFLECT_PROPERTY(m_vert_count)
REFLECT_PROPERTY(m_edges)
REFLECT_END()*/
