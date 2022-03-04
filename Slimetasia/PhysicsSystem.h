#pragma once
#include "Application.h"
#include "DynamicAABBTree.h"
#include "Editor.h"
#include "ISystem.h"
#include "PECollisionDispatch.h"
#include "PhysicsDefs.h"
#include "PhysicsWorldSettings.h"
#include "RigidbodyComponent.h"

class BaseShape;
// forward declarations
class CollisionMesh_2D;
class CollisionMesh_3D;
// class RigidbodyComponent;
// class ReactiveComponent;
struct Ray;
struct RaycastInfo;
struct RaycastData_tmp;

using Collider2DList = std::vector<CollisionMesh_2D*>;
using Collider3DList = std::vector<CollisionMesh_3D*>;
using RigidbodyList = std::vector<RigidbodyComponent*>;
// using ReactiveList = std::vector<ReactiveComponent*>;
using ManifoldList = std::vector<Manifold>;

struct Node;
class PhysicsSystem : public ISystem<PhysicsSystem>
{
public:
    ~PhysicsSystem();
    // funcs
    void Init();
    void Close();
    void Update(const float& dt);
    void Register2DCollider(CollisionMesh_2D* col);
    void Register3DCollider(CollisionMesh_3D* col);
    void RegisterRigidbody(RigidbodyComponent* rigid);
    // void RegisterReactive(ReactiveComponent* react);
    void DrawDebugMeshes();
    void DrawAABBTreeMesh();
    void DeregisterRigidbody(RigidbodyComponent* rigid);
    void Deregister2DCollider(CollisionMesh_2D* col);
    void Deregister3DCollider(CollisionMesh_3D* col);
    // void DeregisterReactive(ReactiveComponent* react);
    void ClearAllLists();
    std::vector<RaycastData_tmp> Raycast(const Ray& ray);
    void UpdateAABBTree(RigidbodyComponent* rigid, AABB& aabb);
    void EditorInsertIntoDTree(RigidbodyComponent* rigid);
    void RepopulateDTree();

    // new funcs

    static PhysicsWorldSettings s_PhyWorldSettings;
    static bool s_PhysicsSystemInitialized;
    static PECollisionDispatch s_ColDispatch;

private:
    friend class ISystem<PhysicsSystem>;

    PhysicsSystem();
    // No copy or move semantics allowed.
    PhysicsSystem(const PhysicsSystem& rhs) = delete;
    PhysicsSystem(PhysicsSystem&& rhs) = delete;
    PhysicsSystem& operator=(const PhysicsSystem& rhs) = delete;
    PhysicsSystem& operator=(PhysicsSystem&& rhs) = delete;

    // void Update2DColliders();
    // void HandleReactiveComponents();
    void Handle3DCollisions();
    void SendEvents();

    Collider2DList m_2DColliderList;
    Collider3DList m_3DColliderList;
    RigidbodyList m_RigidbodyList;
    // ReactiveList m_ReactiveList;
    ManifoldList m_Manifolds;
    ManifoldList m_PrevCollisionEventList;
    ManifoldList m_QueryList;
    std::shared_ptr<Node> m_Octree;

    DynamicAabbTree<RigidbodyComponent> m_DynamicTree;

    // CollisionWorld* m_ColWorld;
};
