#include "CollisionMesh_2D.h"

#include "IntersectionData.h"

void Edge_2D::ComputeNormal()
{
    auto dist = m_edge_vector;
    dist.Normalize();
    m_normal[0] = -dist[1];
    m_normal[1] = dist[0];
}

inline Vector2 Edge_2D::GetEdgeVector() const
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
                    return IntersectionData {};
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
                    return IntersectionData {};
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
                    return IntersectionData {};
            }
            break;
        default:
            std::cout << "The othermesh parameter in CollisionMesh2D_IsColliding is invalid." << std::endl;
            assert(0);
            return IntersectionData {};
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

    firstmax[0] += first->GetWidth() / 2.f;
    firstmax[1] += first->GetHeight() / 2.f;
    firstmin[0] -= first->GetWidth() / 2.f;
    firstmin[1] -= first->GetHeight() / 2.f;

    secondmax[0] += second->GetWidth() / 2.f;
    secondmax[1] += second->GetHeight() / 2.f;
    secondmin[0] -= second->GetWidth() / 2.f;
    secondmin[1] -= second->GetHeight() / 2.f;

    return IntersectionData { ((firstmax[0] >= secondmin[0] && firstmax[0] <= secondmax[0] && firstmax[1] >= secondmin[1] && firstmax[1] <= secondmax[1]) ||
                               (firstmin[0] >= secondmin[0] && firstmin[0] <= secondmax[0] && firstmin[1] >= secondmin[1] && firstmin[1] <= secondmax[1])),
                              0.f, firstpos - secondpos };
}

IntersectionData CollisionMesh_2D::IsCollidingAABBvsCircle(CollisionMesh_2D*& othermesh)
{
    const auto first = dynamic_cast<AABBColliderMesh*>(this);
    const auto second = dynamic_cast<CircleColliderMesh*>(othermesh);
    const auto firstpos = GetPosition() + GetOffset();
    const auto secondpos = second->GetPosition() + second->GetOffset();
    auto firstmax = firstpos, firstmin = firstpos;

    firstmax[0] += first->GetWidth() / 2.f;
    firstmax[1] += first->GetHeight() / 2.f;
    firstmin[0] -= first->GetWidth() / 2.f;
    firstmin[1] -= first->GetHeight() / 2.f;

    const auto nearestpt = second->GetClosestPointInSphere(firstpos);

    return IntersectionData { nearestpt[0] >= firstmin[0] && nearestpt[0] <= firstmax[0] && nearestpt[1] >= firstmin[1] && nearestpt[1] <= firstmax[1], 0.f, firstpos - secondpos };
}

IntersectionData CollisionMesh_2D::IsCollidingAABBvsPolygon(CollisionMesh_2D*& othermesh)
{
    // to be implemented.
    othermesh;
    return IntersectionData {};
}

IntersectionData CollisionMesh_2D::IsCollidingCirclevsAABB(CollisionMesh_2D*& othermesh)
{
    const auto first = dynamic_cast<CircleColliderMesh*>(this);
    const auto second = dynamic_cast<AABBColliderMesh*>(othermesh);
    const auto firstpos = GetPosition() + GetOffset();
    const auto secondpos = second->GetPosition() + second->GetOffset();
    auto secondmax = secondpos, secondmin = secondpos;

    secondmax[0] += second->GetWidth() / 2.f;
    secondmax[1] += second->GetHeight() / 2.f;
    secondmin[0] -= second->GetWidth() / 2.f;
    secondmin[1] -= second->GetHeight() / 2.f;

    const auto nearestpt = first->GetClosestPointInSphere(secondpos);

    return IntersectionData { nearestpt[0] >= secondmin[0] && nearestpt[0] <= secondmax[0] && nearestpt[1] >= secondmin[1] && nearestpt[1] <= secondmax[1], 0.f, firstpos - secondpos };
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

    return IntersectionData { dist <= totalrad, 0.f, firstpos - secondpos };
}

IntersectionData CollisionMesh_2D::IsCollidingCirclevsPolygon(CollisionMesh_2D*& othermesh)
{
    // to be implemented.
    othermesh;
    return IntersectionData {};
}

IntersectionData CollisionMesh_2D::IsCollidingPolygonvsAABB(CollisionMesh_2D*& othermesh)
{
    // to be implemented.
    othermesh;
    return IntersectionData {};
}

IntersectionData CollisionMesh_2D::IsCollidingPolygonvsCircle(CollisionMesh_2D*& othermesh)
{
    // to be implemented.
    othermesh;
    return IntersectionData {};
}

IntersectionData CollisionMesh_2D::IsCollidingPolygonvsPolygon(CollisionMesh_2D*& othermesh)
{
    // to be implemented.
    othermesh;
    return IntersectionData {};
}

bool CollisionMesh_2D::IsCollidingWithMouse(const Vector2& mousepos)
{
    switch (m_mesh_type)
    {
        case CIRCLE: return IsCollidingWithMouseCircle(mousepos); break;

        case AABB_COL_MESH: return IsCollidingWithMouseAABB(mousepos); break;

        case POLYGON: return IsCollidingWithMousePolygon(mousepos); break;

        default: std::cout << "The CollisionMesh's m_mesh_type is invalid." << std::endl; return false;
    }
}

bool CollisionMesh_2D::IsCollidingWithMouseAABB(const Vector2& mousepos)
{
    auto tmp = dynamic_cast<AABBColliderMesh*>(this);
    auto pos = GetPosition() + GetOffset();
    auto max = pos, min = pos;
    max[0] += tmp->GetWidth() / 2.f;
    max[1] += tmp->GetHeight() / 2.f;
    min[0] -= tmp->GetWidth() / 2.f;
    min[1] -= tmp->GetHeight() / 2.f;
    return mousepos[0] >= min[0] && mousepos[0] <= max[0] && mousepos[1] >= min[1] && mousepos[1] <= max[1];
}

bool CollisionMesh_2D::IsCollidingWithMouseCircle(const Vector2& mousepos)
{
    auto tmp = dynamic_cast<CircleColliderMesh*>(this);
    auto pos = GetPosition() + GetOffset();
    auto diff = Vector2 { pos[0] - mousepos[0], pos[1] - mousepos[1] };
    auto radsq = tmp->GetRadius();
    radsq *= radsq;
    auto len = diff.SquareLength();

    return len <= radsq;
}

bool CollisionMesh_2D::IsCollidingWithMousePolygon(const Vector2& mousepos)
{
    // to be implemented.
    return false;
}

Vector2 CircleColliderMesh::GetClosestPointInSphere(const Vector2& pt) const
{
    return (Vector2 { GetPosition() } - pt).Normalized() * m_radius;
}

Vector2 CircleColliderMesh::GetClosestPointInSphere(const Vector3& pt) const
{
    return (Vector2 { GetPosition() - pt }).Normalized() * m_radius;
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
