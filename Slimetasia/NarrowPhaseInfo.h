#pragma once
#include "BaseShape.h"
#include "CorePrerequisites.h"
#include "OverlapPair.h"

struct LastFrameCollisionInfo;
class ContactManifoldInfo;
struct ContactPointInfo;

struct NarrowPhaseInfo
{
    NarrowPhaseInfo(OverlapPair* pair, BaseShape* firstShape, BaseShape* secondShape, MemoryAllocator& allocator);
    ~NarrowPhaseInfo();

    // getters
    LastFrameCollisionInfo* GetLastFrameCollisionData() const { return m_OverlapPair->GetLastFrameCollisionInfo(m_FirstShape->GetCollisionShapeID(), m_SecondShape->GetCollisionShapeID()); }
    Transform* GetFirstTransform() const { return m_FirstShape->GetTransform(); }
    Transform* GetSecondTransform() const { return m_SecondShape->GetTransform(); }

    // funcs
    void AddContactPoint(const Vector3& contactNormal, float penDepth, const Vector3& ptA, const Vector3& ptB);
    void AddContactPointsAsPotentialContactManifold();
    void ResetContacts();

    OverlapPair* m_OverlapPair;
    BaseShape* m_FirstShape;
    BaseShape* m_SecondShape;
    // may need to contain the transforms to map from local space to world space.
    // Transform* m_FirstTransform;
    // Transform* m_SecondTransform;

    ContactPointInfo* m_ContactPoints;
    NarrowPhaseInfo* m_Next;
    MemoryAllocator& m_MemAllocator;
};