#pragma once

#include <iostream>
#include <vector>

#include "AABB.h"
#include "Frustum.h"
#include "Manifold.h"
#include "MathDefs.h"
#include "Ray.h"
#include "RigidbodyComponent.h"
#include "any.h"

#define FATTENING_FACTOR 1.1f

struct Ray;
class Frustum;

using ManifoldList = std::vector<Manifold>;

enum class NodeSide
{
    NONE = 0,
    LEFT,
    RIGHT
};

template <typename T> class DynamicAabbTree
{
public:
    DynamicAabbTree();
    ~DynamicAabbTree();

    using QueryResults = std::vector<T*>;
    using CastResults = std::vector<T*>;

    // Spatial Partition Interface
    void InsertData(T* obj, AABB& aabb);
    void UpdateData(T* obj, AABB& aabb);
    void RemoveData(T* obj);

    void DebugDraw(int level);

    bool Exists(T* obj);

    void CastRay(const Ray& ray, CastResults& results);
    void CastFrustum(const Frustum& frustum, CastResults& results) const;

    // probably used for physics.
    void SelfQuery(ManifoldList& results);

    // static const float mFatteningFactor;

    // Add your implementation here
    struct Node
    {
        Node(T* obj, const AABB& inputAABB)
            : m_AABB(inputAABB)
            , m_ClientData(obj)
        {
            m_AABB.Expand(inputAABB.GetCenter() + inputAABB.GetHalfSize() * FATTENING_FACTOR);
            m_AABB.Expand(inputAABB.GetCenter() - inputAABB.GetHalfSize() * FATTENING_FACTOR);
        }

        Node(const Node& rhs)
            : m_AABB(rhs.m_AABB)
            , m_ClientData(rhs.m_ClientData)
            , m_Left(rhs.m_Left)
            , m_Right(rhs.m_Right)
            , m_Parent(rhs.m_Parent)
            , m_Height(rhs.m_Height)
        {
        }

        // bounding aabb.
        AABB m_AABB = AABB();

        // data.
        T* m_ClientData = nullptr;

        // pointers to the left and right nodes.
        Node* m_Left = nullptr;
        Node* m_Right = nullptr;

        // pointer to the parent node.
        Node* m_Parent = nullptr;

        // distance to deepest leaf.
        int m_Height = 0;

        // side of the parent
        NodeSide m_Side = NodeSide::NONE;

        // only used for frustum tests
        size_t m_lastAxis = 0u;
    };
    void clear();
    Node* m_Data = nullptr;
    std::unordered_map<T*, Node*> m_DataMap;

private:
    Node* SelectNode(Node* data, Node* left, Node* right);
    void Free(Node* node);
    void DebugDrawNode(Node* currNode, int level);
    void UpdateHeight(Node* currNode);
    void UpdateAABB(Node* currNode);
    void BalanceTree(Node* currNode);
    void Raycast(Node* currNode, const Ray& ray, CastResults& results);
    void Frustumcast(Node* currNode, const Frustum& frustum, CastResults& results, bool test = true) const;
    void Query(Node* currNode, ManifoldList& results);
    void Query(Node* first, Node* second, ManifoldList& results);
    void SplitNode(Node* first, Node* second, ManifoldList& results);
};

template <typename T> DynamicAabbTree<T>::DynamicAabbTree() {}

template <typename T> DynamicAabbTree<T>::~DynamicAabbTree()
{
    if (m_Data)
    {
        Free(m_Data);
        m_DataMap.clear();
    }
}

template <typename T> void DynamicAabbTree<T>::InsertData(T* obj, AABB& aabb)
{
    if (m_Data != nullptr)
    {
        Node* currentNode = m_Data;
        bool inserted = false;
        Node* newNode = new Node(obj, aabb);
        m_DataMap.emplace(obj, newNode);

        // insert node in.
        while (!inserted)
        {
            if (currentNode->m_Height == 0)
            {
                Node* leftNode = new Node(*currentNode);
                leftNode->m_Height = 0;
                leftNode->m_Parent = currentNode;
                leftNode->m_Left = nullptr;
                leftNode->m_Right = nullptr;
                leftNode->m_Side = NodeSide::LEFT;
                m_DataMap[leftNode->m_ClientData] = leftNode;

                newNode->m_Parent = currentNode;
                newNode->m_Side = NodeSide::RIGHT;

                currentNode->m_Left = leftNode;
                currentNode->m_Right = newNode;
                currentNode->m_Height = 1;
                currentNode->m_ClientData = m_Data->m_ClientData;
                currentNode->m_AABB = AABB::Combine(leftNode->m_AABB, newNode->m_AABB);
                inserted = true;
            }
            // choose either left or right to insert.
            else
            {
                currentNode = SelectNode(newNode, currentNode->m_Left, currentNode->m_Right);
            }
        }

        UpdateHeight(currentNode);
        BalanceTree(currentNode->m_Parent);
        UpdateAABB(currentNode->m_Parent);
    }
    else
    {
        m_Data = new Node(obj, aabb);
        m_DataMap.emplace(obj, m_Data);
        m_Data->m_Height = 0;
    }
}

template <typename T> void DynamicAabbTree<T>::UpdateData(T* obj, AABB& aabb)
{
    // Warn("Assignment3: Required function un-implemented");
    auto it = m_DataMap.find(obj);
    if (it == m_DataMap.end()) return;

    Node* tmp = it->second;

    if (!(tmp->m_AABB.Contains(aabb)))
    {
        RemoveData(obj);
        InsertData(obj, aabb);
    }
}

template <typename T> void DynamicAabbTree<T>::RemoveData(T* obj)
{
    if (m_Data && m_DataMap.size())
    {
        auto it = m_DataMap.find(obj);
        if (it == m_DataMap.end()) return;

        Node* nodeToRemove = it->second;

        if (nodeToRemove != m_Data)
        {
            Node* parent = nodeToRemove->m_Parent;
            Node* sibling = nodeToRemove->m_Side == NodeSide::LEFT ? parent->m_Right : parent->m_Left;

            if (parent->m_Parent)
            {
                if (parent->m_Side == NodeSide::LEFT)
                    parent->m_Parent->m_Left = sibling;
                else
                    parent->m_Parent->m_Right = sibling;

                sibling->m_Parent = parent->m_Parent;

                if (sibling->m_Left && sibling->m_Right)
                    sibling->m_Height = std::max<int>(sibling->m_Left->m_Height, sibling->m_Right->m_Height) + 1;
                else
                    sibling->m_Height = 0;
                // sibling->m_Left = nullptr;
                // sibling->m_Right = nullptr;
                sibling->m_Side = parent->m_Side;

                delete parent;
                delete nodeToRemove;
                m_DataMap.erase(obj);
                UpdateHeight(sibling->m_Parent);
                BalanceTree(sibling->m_Parent);
                UpdateAABB(sibling->m_Parent);
            }
            else
            {
                m_Data = sibling;
                sibling->m_Parent = nullptr;
                sibling->m_Side = NodeSide::NONE;

                delete parent;
                delete nodeToRemove;
                m_DataMap.erase(obj);
            }
        }
        // if there is only one node.
        else
        {
            delete m_Data;
            m_Data = nullptr;
            m_DataMap.erase(obj);
        }
    }
}

template <typename T> void DynamicAabbTree<T>::DebugDraw(int level)
{
    // Warn("Assignment3: Required function un-implemented");
    if (m_Data) DebugDrawNode(m_Data, level);
}

template <typename T> bool DynamicAabbTree<T>::Exists(T* obj)
{
    if (!m_Data) return false;

    if (m_DataMap.find(obj) != m_DataMap.end()) return true;

    return false;
}

template <typename T> void DynamicAabbTree<T>::CastRay(const Ray& ray, CastResults& results)
{
    // Warn("Assignment3: Required function un-implemented");
    Raycast(m_Data, ray, results);
}

template <typename T> void DynamicAabbTree<T>::CastFrustum(const Frustum& frustum, CastResults& results) const
{
    // Warn("Assignment3: Required function un-implemented");
    Frustumcast(m_Data, frustum, results);
}

template <typename T> void DynamicAabbTree<T>::SelfQuery(ManifoldList& results)
{
    // assert(T == RigidbodyComponent);
    // Warn("Assignment3: Required function un-implemented");
    Query(m_Data, results);
}

template <typename T> typename DynamicAabbTree<T>::Node* DynamicAabbTree<T>::SelectNode(Node* data, Node* left, Node* right)
{
    AABB leftComb = AABB::Combine(data->m_AABB, left->m_AABB);
    AABB rightComb = AABB::Combine(data->m_AABB, right->m_AABB);

    return (abs(leftComb.GetSurfaceArea() - left->m_AABB.GetSurfaceArea()) < abs(rightComb.GetSurfaceArea() - right->m_AABB.GetSurfaceArea()) ? left : right);
}

template <typename T> void DynamicAabbTree<T>::UpdateAABB(Node* currNode)
{
    if (currNode)
    {
        currNode->m_AABB = AABB::Combine(currNode->m_Left->m_AABB, currNode->m_Right->m_AABB);
        UpdateAABB(currNode->m_Parent);
    }
}

template <typename T> void DynamicAabbTree<T>::BalanceTree(Node* currNode)
{
    if (currNode)
    {
        if (currNode->m_Height == 0)
        {
            BalanceTree(currNode->m_Parent);
        }
        else
        {
            int leftHeight = currNode->m_Left->m_Height;
            int rightHeight = currNode->m_Right->m_Height;

            if (abs(leftHeight - rightHeight) > 1)
            {
                Node* oldParent = currNode;
                Node* pivot = leftHeight >= rightHeight ? currNode->m_Left : currNode->m_Right;
                NodeSide oldSide = pivot->m_Side;
                Node* smallChild = pivot->m_Left->m_Height < pivot->m_Right->m_Height ? pivot->m_Left : pivot->m_Right;

                if (oldParent->m_Parent)
                {
                    if (oldParent->m_Side == NodeSide::LEFT)
                        oldParent->m_Parent->m_Left = pivot;
                    else
                        oldParent->m_Parent->m_Right = pivot;

                    pivot->m_Parent = oldParent->m_Parent;
                    pivot->m_Side = oldParent->m_Side;
                }
                else
                {
                    m_Data = pivot;
                    pivot->m_Parent = nullptr;
                    pivot->m_Side = NodeSide::NONE;
                }

                if (smallChild->m_Side == NodeSide::LEFT)
                    pivot->m_Left = oldParent;
                else
                    pivot->m_Right = oldParent;

                oldParent->m_Parent = pivot;
                oldParent->m_Side = smallChild->m_Side;

                if (oldSide == NodeSide::LEFT)
                    oldParent->m_Left = smallChild;
                else
                    oldParent->m_Right = smallChild;

                smallChild->m_Parent = oldParent;
                smallChild->m_Side = oldSide;

                UpdateHeight(oldParent);
                BalanceTree(oldParent);
                UpdateAABB(oldParent);
            }
            else
                BalanceTree(currNode->m_Parent);
        }
    }
}

template <typename T> void DynamicAabbTree<T>::Raycast(Node* currNode, const Ray& ray, CastResults& results)
{
    if (currNode)
    {
        float t = 0.f;

        if (AABB::RayAabb(ray.m_start, ray.m_dir, currNode->m_AABB.GetMin(), currNode->m_AABB.GetMax(), t))
        {
            if (currNode->m_Height)
            {
                Raycast(currNode->m_Left, ray, results);
                Raycast(currNode->m_Right, ray, results);
            }
            else
            {
                results.emplace_back(currNode->m_ClientData, t);
            }
        }
    }
}

template <typename T> void DynamicAabbTree<T>::Frustumcast(Node* currNode, const Frustum& frustum, CastResults& results, bool test) const
{
    if (currNode)
    {
        IntersectionType::Type type;
        size_t tmp = 0;

        if (test) type = Frustum::FrustumAabb(frustum.GetPlanes(), currNode->m_AABB.m_Min, currNode->m_AABB.m_Max, currNode->m_lastAxis);

        if (!test || type != IntersectionType::Outside)
        {
            if (test && type == IntersectionType::Inside) test = false;

            if (currNode->m_Height)
            {
                Frustumcast(currNode->m_Left, frustum, results, test);
                Frustumcast(currNode->m_Right, frustum, results, test);
            }
            else
            {
                results.emplace_back(currNode->m_ClientData);
            }
        }
    }
}

template <typename T> void DynamicAabbTree<T>::Query(Node* currNode, ManifoldList& results)
{
    if (!currNode->m_Height) return;

    Query(currNode->m_Left, results);
    Query(currNode->m_Right, results);
    Query(currNode->m_Left, currNode->m_Right, results);
}

template <typename T> void DynamicAabbTree<T>::Query(Node* first, Node* second, ManifoldList& results)
{
    if (AABB::AABBAABB(first->m_AABB.m_Min, first->m_AABB.m_Max, second->m_AABB.m_Min, second->m_AABB.m_Max))
    {
        if ((first->m_Height == 0) && (second->m_Height == 0))
            results.emplace_back(first->m_ClientData, second->m_ClientData);
        else
            SplitNode(first, second, results);
    }
}

template <typename T> void DynamicAabbTree<T>::SplitNode(Node* first, Node* second, ManifoldList& results)
{
    if (!second->m_Height)
        std::swap(first, second);
    else if (first->m_Height && second->m_Height)
    {
        if (first->m_AABB.GetVolume() > second->m_AABB.GetVolume()) std::swap(first, second);
    }

    Query(first, second->m_Left, results);
    Query(first, second->m_Right, results);
}

template <typename T> inline void DynamicAabbTree<T>::clear()
{
    if (m_Data)
    {
        Free(m_Data);
        m_Data = nullptr;
        m_DataMap.clear();
    }
}

template <typename T> void DynamicAabbTree<T>::Free(Node* node)
{
    if (node->m_Left)
    {
        Free(node->m_Left);
        node->m_Left = nullptr;
    }
    if (node->m_Right)
    {
        Free(node->m_Right);
        node->m_Right = nullptr;
    }

    delete (node);
}

template <typename T> void DynamicAabbTree<T>::DebugDrawNode(Node* currNode, int level)
{
    currNode->m_AABB.DebugDraw(currNode->m_ClientData->GetOwner()->GetParentLayer()->GetId());

    if (currNode->m_Height > 0)
    {
        DebugDrawNode(currNode->m_Left, level + 1);
        DebugDrawNode(currNode->m_Right, level + 1);
    }
}

template <typename T> void DynamicAabbTree<T>::UpdateHeight(Node* currNode)
{
    if (currNode)
    {
        currNode->m_Height = currNode->m_Left->m_Height > currNode->m_Right->m_Height ? currNode->m_Left->m_Height + 1 : currNode->m_Right->m_Height + 1;
        UpdateHeight(currNode->m_Parent);
    }
}
