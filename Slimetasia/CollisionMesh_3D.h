#pragma once

#include <thread>

#include "AABB.h"
#include "CorePrerequisites.h"
#include "GameObject.h"
#include "IComponent.h"
#include "IntersectionData.h"
#include "PhysicsDefs.h"
#include "PhysicsSystem.h"
#include "Ray.h"
#include "RaycastInfo.h"
#include "Transform.h"

#define discpoints 16u
/*#define PI 3.14159265358979323846
#define PI_F static_cast<float>(PI)
#define DITHER PI_F * 2.f / static_cast<float>(discpoints)*/

float constexpr DITHER = RAD180 * 2.f / static_cast<float>(discpoints);

// struct OctreeData
//{
//  Node* pNode;
//  Vector3 newCenter;
//  float newWidth;
//  int newDepth;
//  int id;
//};
//
// struct CollisionInfoIn
//{
//  GameObject * obj1;
//  GameObject * obj2;
//};
//
// struct CollisionRayIn
//{
//	GameObject* obj1;
//	Ray ray;
//};
//
// enum COLLISIONMESHTYPE_3D
//{
//  COL_POINT = 0,
//  SPHERE,			//3D circles
//  CUBOID,			//3D AABBs
//  PLANE,
//  RAY,
//  TRIANGLE,
//  FRUSTUM
//};

class CollisionMesh_3D : public IComponent
{
public:

    // new constructor
    CollisionMesh_3D(GameObject* parentObject, const std::string& name = "CollisionMesh_3D", const CollisionShapeType& shapetype = eCollisionShapeType_SPHERE,
                     const CollisionShape& shape = eCollisionShape_TRIANGLE, const float& rad = 1.f);

    ~CollisionMesh_3D()
    {
        if (m_OwnerObject)
        {
            GetInstance(PhysicsSystem).Deregister3DCollider(this);
            // std::cout << "CollisionMesh_3D death!\n";
        }
    }

    // getters
    // virtual IntersectionList& GetIntersectionList() { return m_intersectList; }
    virtual Vector3 GetOffset() const { return m_offset; }
    virtual Vector3 GetPosition() const { return m_OwnerObject->GetComponent<Transform>()->GetWorldPosition(); }
    virtual float GetRadius() const { return m_radius; }

    // setters
    virtual void SetOffset(const Vector3& offset) { m_offset = offset; }
    virtual void SetPosition(const Vector3& newPos);

    // funcs
    // virtual IntersectionData IsColliding(CollisionMesh_3D* othermesh);
    // virtual IntersectionData IsCollidingCuboidvsCuboid(CollisionMesh_3D* & othermesh);
    // virtual IntersectionData IsCollidingCuboidvsSphere(CollisionMesh_3D* & othermesh);
    // virtual IntersectionData IsCollidingCuboidvsPolyhedra(CollisionMesh_3D* & othermesh);
    // virtual IntersectionData IsCollidingSpherevsCuboid(CollisionMesh_3D* & othermesh);
    // virtual IntersectionData IsCollidingSpherevsSphere(CollisionMesh_3D* & othermesh);
    // virtual IntersectionData IsCollidingSpherevsPolyhedra(CollisionMesh_3D* & othermesh);
    // virtual IntersectionData IsCollidingPolyhedravsCuboid(CollisionMesh_3D* & othermesh);
    // virtual IntersectionData IsCollidingPolyhedravsSphere(CollisionMesh_3D* & othermesh);
    // virtual IntersectionData IsCollidingPolyhedravsPolyhedra(CollisionMesh_3D* & othermesh);
    virtual bool ContainsPoint(const Vector3& pt);

    virtual bool Raycast(const Ray& ray, RaycastData_tmp& data);

    virtual void ComputeBounds(Vector3& min, Vector3& max) const;

    // virtual bool IsCollidingWithMouse(const Vector2& mousepos);
    // virtual bool IsCollidingWithMouseCuboid(const Vector2& mousepos);
    // virtual bool IsCollidingWithMouseSphere(const Vector2& mousepos);
    // virtual bool IsCollidingWithMousePolyhedra(const Vector2& mousepos);

    virtual void Register();
    virtual void DebugDraw();

    // virtual void EnableLayerCollision(const unsigned& layer);
    // virtual void DisableLayerCollision(const unsigned& layer);

    virtual void AddIntersection(const IntersectionData& data);
    // void ComputeRadius();

    void SendMessages();
    IntersectionList* GetPrevList() { return &m_prevIntersect; }
    IntersectionList* GetCurrentList() { return &m_currentIntersect; }

    // new stuff
    void ComputeMass();
    virtual void ComputeInertiaTensor(Matrix3& tensor, const float& mass) const;
    CollisionShapeType GetCollisionShapeType() const { return m_ColShapeType; }
    CollisionShape GetCollisionShape() const { return m_ColShape; }

    REFLECT()
protected:

    // char m_collision_layer = 0x0;
    Vector3 m_offset {};
    IntersectionList m_intersectList;
    IntersectionList m_prevIntersect;
    IntersectionList m_currentIntersect;
    // COLLISIONMESHTYPE_3D m_mesh_type = SPHERE;
    float m_radius;

    // new stuff
    CollisionShapeType m_ColShapeType;
    CollisionShape m_ColShape;
    // float m_Margin;
};

// a 3D point is a sphere collider with 0 radius.
// class SphereColliderMesh : public CollisionMesh_3D
//{
// public:
//  SphereColliderMesh(GameObject* parent = nullptr, const float& rad = 1.f)
//    : CollisionMesh_3D{ parent, "SphereColliderMesh" , SPHERE, rad }
//  {
//
//  }
//  //getters
//  //float GetRadius()const { return m_radius; }
//  float GetRadiusSq()const { return m_radius * m_radius; }
//
//  //setters
//  void SetRadius(const float& new_rad) { m_radius = new_rad; }
//
//  //func
//  Vector3 GetClosestPointInSphere(const Vector2& pt) const;
//  Vector3 GetClosestPointInSphere(const Vector3& pt) const;
//  bool Raycast(const Ray& ray, RaycastData_tmp& data) override;
//  bool ContainsPoint(const Vector3& pt) override;
//  void DebugDraw()override;
//  //REFLECT()
// private:
//  //float m_radius = 0.f;
//};
//
// class CuboidColliderMesh : public CollisionMesh_3D
//{
// public:
//  CuboidColliderMesh(GameObject* parent = nullptr, const float& height = 1.f, const float& width = 1.f, const float& depth = 1.f)
//    : CollisionMesh_3D{ parent, "CuboidColliderMesh", CUBOID }
//  {
//    if (parent)
//      ComputeRadius();
//  }
//
//  //getters
//  float GetHeight()const { return m_height; }
//  float GetWidth()const { return m_width; }
//  float GetDepth()const { return m_depth; }
//
//  //setters
//  void SetHeight(const float& new_height) { m_height = new_height; ComputeRadius(); }
//  void SetWidth(const float& new_width) { m_width = new_width; ComputeRadius(); }
//  void SetDepth(const float& new_depth) { m_depth = new_depth; ComputeRadius(); }
//
//  //func
//  Vector3 GetClosestPointInCuboid(const Vector2& pt) const;
//  Vector3 GetClosestPointInCuboid(const Vector3& pt) const;
//  float GetLongestEdge()const;
//  void DebugDraw() override;
//  int VectorDir(const Vector3& pt)const;
//  bool Raycast(const Ray& ray, RaycastData_tmp& data) override;
//  bool ContainsPoint(const Vector3& pt) override;
//  static Vector3 s_Compass[6];
//  //REFLECT()
// private:
//  float m_height = 0.f, m_width = 0.f, m_depth = 0.f;
//
//};

// void             CollisionCheck(void* in);
// void			 RaycastCheck(void* in, std::vector<RaycastData_tmp>& vec);
