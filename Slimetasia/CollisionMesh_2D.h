#pragma once

#include "CorePrerequisites.h"
#include "GameObject.h"
#include "IComponent.h"
#include "PhysicsSystem.h"
#include "Transform.h"

struct IntersectionData;

enum COLLISIONMESHTYPE_2D
{
    CIRCLE = 0,     // 2D circles and mouse point
    AABB_COL_MESH,  // 2D AABB
    POLYGON         // 2D polygons
};

struct Edge_2D
{
    Edge_2D() = default;
    Edge_2D(const Vector2& startpt, const Vector2& endpt)
        : m_start_pt { startpt }
        , m_end_pt { endpt }
        , m_edge_vector { m_end_pt - m_start_pt }
    {
        ComputeNormal();
    }

    void ComputeNormal();
    inline Vector2 GetEdgeVector() const;

    Vector2 m_normal;
    Vector2 m_start_pt;
    Vector2 m_end_pt;
    Vector2 m_edge_vector;
};

// Dependent on the object having a Transform Component.
class CollisionMesh_2D : public IComponent
{
public:

    CollisionMesh_2D(GameObject* parentObject = nullptr, const std::string& name = "CollisionMesh_2D")
        : IComponent(parentObject, name)
    {
        if (parentObject)
        {
            parentObject->AddIfDoesntExist<Transform>();
            GetInstance(PhysicsSystem).Register2DCollider(this);
            // m_Registered = true;
        }
    }

    ~CollisionMesh_2D() {}

    virtual IntersectionData IsColliding(CollisionMesh_2D*& othermesh);
    virtual IntersectionData IsCollidingAABBvsAABB(CollisionMesh_2D*& othermesh);
    virtual IntersectionData IsCollidingAABBvsCircle(CollisionMesh_2D*& othermesh);
    virtual IntersectionData IsCollidingAABBvsPolygon(CollisionMesh_2D*& othermesh);
    virtual IntersectionData IsCollidingCirclevsAABB(CollisionMesh_2D*& othermesh);
    virtual IntersectionData IsCollidingCirclevsCircle(CollisionMesh_2D*& othermesh);
    virtual IntersectionData IsCollidingCirclevsPolygon(CollisionMesh_2D*& othermesh);
    virtual IntersectionData IsCollidingPolygonvsAABB(CollisionMesh_2D*& othermesh);
    virtual IntersectionData IsCollidingPolygonvsCircle(CollisionMesh_2D*& othermesh);
    virtual IntersectionData IsCollidingPolygonvsPolygon(CollisionMesh_2D*& othermesh);

    virtual bool IsCollidingWithMouse(const Vector2& mousepos);
    virtual bool IsCollidingWithMouseAABB(const Vector2& mousepos);
    virtual bool IsCollidingWithMouseCircle(const Vector2& mousepos);
    virtual bool IsCollidingWithMousePolygon(const Vector2& mousepos);

    // getters
    virtual Vector3 GetOffset() const { return m_offset; }
    virtual Vector3 GetPosition() const { return m_OwnerObject->GetComponent<Transform>()->GetWorldPosition(); }

    // setters
    virtual void SetOffset(const Vector3& offset) { m_offset = offset; }

    // data
    COLLISIONMESHTYPE_2D m_mesh_type = CIRCLE;
    // void Unregister() override;

    REFLECT()
protected:

    Vector3 m_offset;

private:
};

// a 2D point is a circle collider with 0 radius.
class CircleColliderMesh : public CollisionMesh_2D
{
public:

    CircleColliderMesh(GameObject* parentobject, const float& rad = 0.f)
        : CollisionMesh_2D { parentobject, "CircleColliderMesh" }
        , m_radius { rad }
    {
    }

    // getters
    float GetRadius() const { return m_radius; }

    // setters
    void SetRadius(const float& new_rad) { m_radius = new_rad; }

    // func
    Vector2 GetClosestPointInSphere(const Vector2& pt) const;
    Vector2 GetClosestPointInSphere(const Vector3& pt) const;

    REFLECT()
private:

    float m_radius;
};

class AABBColliderMesh : public CollisionMesh_2D
{
public:

    AABBColliderMesh(GameObject* parentobject, const float& height = 0.f, const float& width = 0.f)
        : CollisionMesh_2D { parentobject, "AABBColliderMesh" }
        , m_height { height }
        , m_width { width }
    {
    }

    // getters
    float GetHeight() const { return m_height; }
    float GetWidth() const { return m_width; }

    // setters
    void SetHeight(const float& new_height) { m_height = new_height; }
    void SetWidth(const float& new_width) { m_width = new_width; }

    REFLECT()
private:

    float m_height, m_width;
};

class POLYGONColliderMesh : public CollisionMesh_2D
{
public:

    POLYGONColliderMesh(GameObject* parentobject, const int& verts = 0, const std::vector<Edge_2D>& edges = std::vector<Edge_2D>())
        : CollisionMesh_2D { parentobject, "POLYGONColliderMesh" }
        , m_vert_count { verts }
        , m_edges { edges }
    {
    }
    int GetNumVerts() const { return m_vert_count; }
    std::vector<Edge_2D> GetEdges() const { return m_edges; }

    // REFLECT()
private:

    int m_vert_count;
    std::vector<Edge_2D> m_edges;
};