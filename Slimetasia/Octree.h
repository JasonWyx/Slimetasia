#pragma once
#include "CollisionMesh_3D.h"
#include "GameObject.h"
#include "Transform.h"

struct Node
{
    // For Creation of Bouding Box
    Vector3 m_center;
    float m_halfWidth;
    std::shared_ptr<Node> m_Child[8];  // child nodes per parent node
    GameObject* m_ObjList;             // Objects in the contained in the node

    Node()
        : m_center()
        , m_halfWidth()
        , m_Child()
        , m_ObjList(nullptr)
    {
    }
};
std::shared_ptr<Node> ConstructOctree(Vector3 center, float halfWidth, int depth);
// Insert into octree
void InsertObject(std::shared_ptr<Node> pTree, GameObject* pObj);
// Test collision for all the objects in the tree
void CollisionTestTree(std::shared_ptr<Node> pTree);
void RayTestTree(std::shared_ptr<Node> pTree, std::vector<RaycastData_tmp>& vec);
// void DeleteTree(Node * pTree);
std::shared_ptr<Node> OctreeCreate();
void DeleteColInfo();
// void DeleteRayInfo();
// static std::vector<CollisionInfoIn*> inData;
// static std::vector<OctreeData*> octreeData;
void Reset();
// static CollisionInfoIn* col_in;//= new CollisionInfoIn[1000];