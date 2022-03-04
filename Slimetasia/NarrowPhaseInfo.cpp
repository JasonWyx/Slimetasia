#include "NarrowPhaseInfo.h"

#include <assert.h>

#include "ContactPointInfo.h"
#include "TriangleShape.h"

NarrowPhaseInfo::NarrowPhaseInfo(OverlapPair* pair, BaseShape* firstShape, BaseShape* secondShape, MemoryAllocator& allocator)
    : m_OverlapPair(pair)
    , m_FirstShape(firstShape)
    , m_SecondShape(secondShape)
    , m_ContactPoints(nullptr)
    , m_MemAllocator(allocator)
    , m_Next(nullptr)
{
    m_OverlapPair->AddLastFrameInfoIfNecessary(m_FirstShape->GetCollisionShapeID(), m_SecondShape->GetCollisionShapeID());
}

NarrowPhaseInfo::~NarrowPhaseInfo()
{
    assert(!m_ContactPoints);

    if (m_FirstShape->GetCollisionShape() == eCollisionShape_TRIANGLE)
    {
        m_FirstShape->~BaseShape();
        m_MemAllocator.ReleaseMemory(m_FirstShape, sizeof(TriangleShape));
    }

    if (m_SecondShape->GetCollisionShape() == eCollisionShape_TRIANGLE)
    {
        m_SecondShape->~BaseShape();
        m_MemAllocator.ReleaseMemory(m_SecondShape, sizeof(TriangleShape));
    }
}

void NarrowPhaseInfo::AddContactPoint(const Vector3& contactNormal, float penDepth, const Vector3& ptA, const Vector3& ptB)
{
    assert(penDepth > 0.f);

    MemoryAllocator& allocator = m_OverlapPair->GetTempAllocator();

    ContactPointInfo* info = new (allocator.Allocate(sizeof(ContactPointInfo))) ContactPointInfo(contactNormal, penDepth, ptA, ptB);

    info->m_Next = m_ContactPoints;

    m_ContactPoints = info;
}

void NarrowPhaseInfo::AddContactPointsAsPotentialContactManifold()
{
    m_OverlapPair->AddPotentialContactPts(this);
}

void NarrowPhaseInfo::ResetContacts()
{
    MemoryAllocator& allocator = m_OverlapPair->GetTempAllocator();

    ContactPointInfo* elem = m_ContactPoints;

    while (elem)
    {
        ContactPointInfo* elemToDelete = elem;

        elem = elem->m_Next;

        elemToDelete->~ContactPointInfo();

        allocator.ReleaseMemory(elemToDelete, sizeof(ContactPointInfo));
    }

    m_ContactPoints = nullptr;
}
