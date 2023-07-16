#include "Octree.h"

#include "Renderer.h"
#include "ThreadPool.h"

std::shared_ptr<Node> ConstructOctree(Vector3 center, float halfWidth, int depth)
{
    // depth specified to limit subdivision
    if (depth < 0)
        return nullptr;
    else
    {
        // Construct Root Node of subtree
        auto pNode = std::shared_ptr<Node>(new Node);

        pNode->m_center = center;
        pNode->m_halfWidth = halfWidth;
        pNode->m_ObjList = nullptr;
        // Recursion Call to construct children
        Vector3 offset;
        float subWidth = halfWidth * 0.5f;
        // Construct 8 permutations of subWidth towards center
        for (int i = 0; i < 8; ++i)
        {
            pNode->m_Child[i] = nullptr;
            offset.x = ((i & 1) ? subWidth : -subWidth);
            offset.y = ((i & 2) ? subWidth : -subWidth);
            offset.z = ((i & 4) ? subWidth : -subWidth);
            pNode->m_Child[i] = ConstructOctree(center + offset, subWidth, depth - 1);
        }
        return pNode;
    }
}

void InsertObject(std::shared_ptr<Node> pTree, GameObject* pObj)
{
    int index = 0;
    bool straddle = false;
    auto col = pObj->GetComponent<CollisionMesh_3D>();
    auto trans = pObj->GetComponent<Transform>();
    // Check for x y z
    for (int i = 0; i < 3; ++i)
    {
        if (col)
        {
            float delta = trans->GetWorldPosition()[i] - pTree->m_center[i];

            if (abs(delta) < col->GetRadius())
            {
                straddle = true;
                break;
            }
            if (delta > 0.f) index |= (1 << i);  // ZYX planes will shift to the affected octant
        }
    }

    // Non straddle, recursive call to traverse down the tree
    if (!straddle)
    {
        if (pTree->m_Child[index] == nullptr)
            straddle = true;
        else
            InsertObject(pTree->m_Child[index], pObj);
    }
    if (straddle)
    {
        // straddling, link object into linked list at this node

        pObj->m_NextObj = pTree->m_ObjList;
        pTree->m_ObjList = pObj;
    }

#ifdef USE_VULKAN
#else
    Renderer::Instance().DrawCube(pTree->m_halfWidth, pTree->m_center);
#endif  // USE_VULKAN

    auto pid = pObj->GetParentLayer()->GetId();

#ifdef USE_VULKAN
    auto currentLayerID = 0U;  // todo
#else
    auto currentLayerID = Renderer::Instance().GetCurrentEditorLayer()->GetId();
#endif  // USE_VULKAN

    if (pid != currentLayerID)
    {
        return;
    }
}
static unsigned i_index = 0;
void CollisionTestTree(std::shared_ptr<Node> pTree)
{
    const int DEPTHMAX = 40;
    // For storing the whole tree and referencing
    static std::shared_ptr<Node> parentStack[DEPTHMAX];
    // How deep the tree has traversed
    static int depth = 0;
    parentStack[depth++] = pTree;
    // for (int n = 0; n < depth; ++n)
    //{
    //  GameObject * pA, *pB;
    //  //Go through the object list in the current node
    //  for (pA = parentStack[n]->m_ObjList; pA; pA = pA->m_NextObj)
    //  {
    //	  for (pB = pTree->m_ObjList; pB; pB = pB->m_NextObj)
    //	  {
    //		  if (pA == pB) break;//skip same object test
    //		  IntersectionData data = pA->GetComponent<CollisionMesh_3D>()->IsColliding(pB->GetComponent<CollisionMesh_3D>());
    //		  if (data.m_is_intersect)
    //		  {
    //        data.m_FirstRigidbody = pA->GetComponent<RigidbodyComponent>();
    //        data.m_SecondRigidbody = pB->GetComponent<RigidbodyComponent>();
    //
    //			  pA->GetComponent<CollisionMesh_3D>()->AddIntersection(data);
    //
    //			  pB->GetComponent<CollisionMesh_3D>()->AddIntersection(-(data));
    //		  }
    //      //Multi-threading
    //      //CollisionInfoIn * inObject = col_in + i_index++;
    //      //inObject->obj1 = pA;
    //      //inObject->obj2 = pB;
    //      ////bind pointer to member function
    //      //auto fn = std::bind(&CollisionCheck, inObject);
    //      ////Insert to threadpool
    //      //tp.InsertJob(fn);
    //
    //    }
    //  }
    //}
    // Traverse to all child nodes
    for (int i = 0; i < 8; ++i)
    {
        if (pTree->m_Child[i]) CollisionTestTree(pTree->m_Child[i]);
    }
    // Remove current node from stack
    depth--;
}

// void DeleteTree(Node * pTree)
//{
//  //if (pTree == nullptr) return;
//	for (int i = 0; i < 8; ++i)
//	{
//    if (pTree->m_Child[i])
//    {
//      DeleteTree(pTree->m_Child[i]);
//    }
//	}
//	delete pTree;
//  pTree = nullptr;
//}

std::shared_ptr<Node> OctreeCreate()
{
    return ConstructOctree(Vector3(0, 0, 0), 50.f, 3);
}

void Reset()
{
    i_index = 0;
}

void DeleteColInfo()
{
    // delete[] col_in;
}
