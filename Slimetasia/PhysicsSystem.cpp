#include "PhysicsSystem.h"

#include "CollisionMesh_2D.h"
#include "CollisionMesh_3D.h"
#include "GameObject.h"
#include "Input.h"
//#include "RigidbodyComponent.h"
#include "Manifold.h"
#include "Octree.h"
#include "PEList.h"
#include "Ray.h"
#include "RaycastInfo.h"
#include "ReactiveComponent.h"

PhysicsWorldSettings PhysicsSystem::s_PhyWorldSettings;
bool PhysicsSystem::s_PhysicsSystemInitialized = false;
PECollisionDispatch PhysicsSystem::s_ColDispatch;

PhysicsSystem::~PhysicsSystem()
{
    // DeleteColInfo();
}

void PhysicsSystem::Init()
{
    s_PhyWorldSettings.LoadFromFile();

    for (auto& elem : m_RigidbodyList)
    {
        elem->Initialize();
        // elem->EditorInitialize();
    }

    s_PhysicsSystemInitialized = true;
}

void PhysicsSystem::Close()
{
    // write back into the settings file.
    // s_PhyWorldSettings.SaveToFile();

    s_PhysicsSystemInitialized = false;
    m_Manifolds.clear();
    m_PrevCollisionEventList.clear();
    m_DynamicTree.clear();
}

void PhysicsSystem::Update(const float& dt)
{
#ifdef EDITOR
    DrawDebugMeshes();
    // DrawAABBTreeMesh();
#endif

    float mydt = dt > (1.0f / 30) ? (1.0f / 30) : dt;

    if (dt == 0.f) return;

    // std::cout << "dynamicTree size = " << m_DynamicTree.m_DataMap.size() << "\n";
    // std::cout << "rigidbody list size = " << m_RigidbodyList.size() << "\n";

    if (s_PhysicsSystemInitialized)
    {
        m_PrevCollisionEventList = m_Manifolds;
        m_Manifolds.clear();
        m_Manifolds.reserve(m_RigidbodyList.size() * m_RigidbodyList.size());
        m_QueryList.clear();
        m_QueryList.reserve(m_RigidbodyList.size() * m_RigidbodyList.size());

        if (m_DynamicTree.m_Data) m_DynamicTree.SelfQuery(m_QueryList);

        // for (auto i = 0u; i < m_RigidbodyList.size(); i++)
        //{
        //  for (auto j = i + 1; j < m_RigidbodyList.size(); j++)
        //  {
        //  	//check if either object has an infinite mass.
        //  	if (m_RigidbodyList[i]->m_InverseMass == 0.f || m_RigidbodyList[j]->m_InverseMass == 0.f)
        //      continue;
        //
        //  	Manifold tmp{m_RigidbodyList[i], m_RigidbodyList[j]};
        //  	tmp.Solve();
        //
        //  	if(tmp.m_ContactCount)
        //      m_Manifolds.emplace_back(tmp);
        //  }
        //}

        for (auto& elem : m_QueryList)
        {
            elem.Solve();

            if (elem.m_ContactCount) m_Manifolds.emplace_back(elem);
        }

        for (auto i = 0u; i < m_RigidbodyList.size(); i++)
            m_RigidbodyList[i]->IntegrateForces(mydt);

        for (auto i = 0u; i < m_Manifolds.size(); i++)
            m_Manifolds[i].Initialize();

        for (uint count = 0u; count < s_PhyWorldSettings.m_DefaultVelSolverIteration; count++)
            for (auto i = 0u; i < m_Manifolds.size(); i++)
                m_Manifolds[i].ApplyImpulse();

        for (auto i = 0u; i < m_RigidbodyList.size(); i++)
            m_RigidbodyList[i]->IntegrateVelocity(mydt);

        for (uint count = 0u; count < s_PhyWorldSettings.m_DefaultPosSolverIteration; count++)
            for (auto i = 0u; i < m_Manifolds.size(); i++)
                m_Manifolds[i].PositionCorrection();

        for (auto i = 0u; i < m_RigidbodyList.size(); i++)
            m_RigidbodyList[i]->m_Force.Zero();

        SendEvents();
    }
}

void PhysicsSystem::Register2DCollider(CollisionMesh_2D* col)
{
    m_2DColliderList.push_back(col);
}

void PhysicsSystem::Register3DCollider(CollisionMesh_3D* col)
{
    m_3DColliderList.push_back(col);

    auto parentObject = col->GetOwner();
    auto rigid = parentObject->GetComponent<RigidbodyComponent>();

    if (rigid)
    {
        if (!m_DynamicTree.Exists(rigid))
        {
            AABB rigid_aabb(rigid);
            m_DynamicTree.InsertData(rigid, rigid_aabb);
        }
        else
        {
            if (!rigid->m_3DColliders.empty())
            {
                m_DynamicTree.RemoveData(rigid);
                AABB rigid_aabb(rigid);
                m_DynamicTree.InsertData(rigid, rigid_aabb);
            }
        }
        rigid->m_3DColliders.emplace_back(col);
    }
}

void PhysicsSystem::RegisterRigidbody(RigidbodyComponent* rigid)
{
    m_RigidbodyList.push_back(rigid);

    auto parentObject = rigid->GetOwner();
    rigid->m_3DColliders = parentObject->GetAllComponentWithThisBase<CollisionMesh_3D>();
    ;

    if (!rigid->m_3DColliders.empty())
    {
        if (!m_DynamicTree.Exists(rigid))
        {
            AABB rigid_aabb(rigid);
            m_DynamicTree.InsertData(rigid, rigid_aabb);
        }
    }
}

void PhysicsSystem::DrawDebugMeshes()  // For now draws 3d bounding box and sphere only.
{
    if (!Application::s_IsGameRunning && !Editor::s_ShowDebug)
    {
        if (m_3DColliderList.size() > 0)
        {
            for (auto& elem : m_3DColliderList)
                elem->DebugDraw();
        }
        // s_DynamicsWorld.DebugDraw();
    }
}

void PhysicsSystem::DrawAABBTreeMesh()
{
    m_DynamicTree.DebugDraw(0);

    // std::cout << "Total number of rigidbodies : " << m_RigidbodyList.size() << "\n";
    // std::cout << "Total number of items in the tree : " << m_DynamicTree.m_DataMap.size() << "\n";
}

void PhysicsSystem::Deregister2DCollider(CollisionMesh_2D* col)
{
    m_2DColliderList.erase(std::remove(m_2DColliderList.begin(), m_2DColliderList.end(), col), m_2DColliderList.end());
}

void PhysicsSystem::Deregister3DCollider(CollisionMesh_3D* col)
{
    // const auto del = std::find(m_3DColliderList.begin(), m_3DColliderList.end(), col);
    // if (del != m_3DColliderList.end()) m_3DColliderList.erase(del);
    m_3DColliderList.erase(std::remove(m_3DColliderList.begin(), m_3DColliderList.end(), col), m_3DColliderList.end());
}

PhysicsSystem::PhysicsSystem()
{
    // m_Octree = OctreeCreate();
    // read from the settings file.
#ifndef EDITOR
    Init();
#endif  // !EDITOR
}

void PhysicsSystem::DeregisterRigidbody(RigidbodyComponent* rigid)
{
    for (auto& elem : m_Manifolds)
    {
        if (elem.m_FirstBody == rigid) elem.m_FirstBody = nullptr;

        if (elem.m_SecondBody == rigid) elem.m_SecondBody = nullptr;
    }

    const auto del = std::find(m_RigidbodyList.begin(), m_RigidbodyList.end(), rigid);
    if (del != m_RigidbodyList.end()) m_RigidbodyList.erase(del);

    m_DynamicTree.RemoveData(rigid);
}

void PhysicsSystem::ClearAllLists()
{
    for (auto& elem : m_3DColliderList)
    {
        elem->SendMessages();
        // elem->GetIntersectionList().clear();
    }
}

std::vector<RaycastData_tmp> PhysicsSystem::Raycast(const Ray& ray)
{
    std::vector<RaycastData_tmp> data;

    for (auto& elem : m_3DColliderList)
    {
        RaycastData_tmp tmp;
        if (elem->Raycast(ray, tmp)) data.push_back(tmp);
    }

    return data;
}

void PhysicsSystem::UpdateAABBTree(RigidbodyComponent* rigid, AABB& aabb)
{
    m_DynamicTree.UpdateData(rigid, aabb);
}

void PhysicsSystem::EditorInsertIntoDTree(RigidbodyComponent* rigid)
{
    AABB aabb(rigid);
    m_DynamicTree.InsertData(rigid, aabb);
}

void PhysicsSystem::RepopulateDTree()
{
    for (auto& elem : m_RigidbodyList)
        elem->EditorInitialize();
}

void PhysicsSystem::Handle3DCollisions()
{
    auto max = m_3DColliderList.size();
    if (m_3DColliderList.size())
    {
        for (auto& elem : m_3DColliderList)
        {
            InsertObject(m_Octree, elem->GetOwner());
        }
        CollisionTestTree(m_Octree);
        Reset();
    }
}

void PhysicsSystem::SendEvents()
{
    // checking exit and oncollide
    for (auto& elem : m_PrevCollisionEventList)
    {
        if (elem.m_FirstBody == nullptr || elem.m_SecondBody == nullptr) continue;

        auto firstParent = elem.m_FirstBody->GetOwner();
        auto firstScripts = firstParent->GetLuaScripts();
        auto secondParent = elem.m_SecondBody->GetOwner();
        auto secondScripts = secondParent->GetLuaScripts();

        // if the data doesnt exist in the current frame
        if (std::find(m_Manifolds.begin(), m_Manifolds.end(), elem) == m_Manifolds.end())
        {
            if (elem.m_SecondBody)
            {
                for (auto& s : firstScripts)
                {
                    s->OnCollisionExit(secondParent);
                }
            }

            if (elem.m_FirstBody)
            {
                for (auto& s : secondScripts)
                {
                    s->OnCollisionExit(firstParent);
                }
            }
        }
        // if the data still exist in the current frame.
        else
        {
            if (elem.m_SecondBody)
            {
                for (auto& s : firstScripts)
                {
                    s->OnCollisionPersist(secondParent);
                }
            }

            if (elem.m_FirstBody)
            {
                for (auto& s : secondScripts)
                {
                    s->OnCollisionPersist(firstParent);
                }
            }
        }
    }

    for (auto& elem : m_Manifolds)
    {
        // if the data is new
        if (std::find(m_PrevCollisionEventList.begin(), m_PrevCollisionEventList.end(), elem) == m_PrevCollisionEventList.end())
        {
            auto scripts = elem.m_FirstBody->GetOwner()->GetLuaScripts();
            for (auto& s : scripts)
            {
                s->OnCollisionEnter(elem.m_SecondBody->GetOwner());
            }

            scripts = elem.m_SecondBody->GetOwner()->GetLuaScripts();
            for (auto& s : scripts)
            {
                s->OnCollisionEnter(elem.m_FirstBody->GetOwner());
            }
        }
    }
}
