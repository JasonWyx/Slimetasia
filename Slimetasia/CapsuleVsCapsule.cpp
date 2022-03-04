#include "CapsuleVsCapsule.h"

#include <cassert>

#include "CapsuleCollider.h"
#include "Manifold.h"
#include "RigidbodyComponent.h"

void CapsuleVsCapsule::ComputeCollisionData(Manifold* manifold, RigidbodyComponent* first, RigidbodyComponent* second)
{
    // ensuring all parameters are valid.
    assert(manifold != nullptr && first != nullptr && second != nullptr);

    // ensuring all parameters are valid.
    assert(manifold != nullptr && first != nullptr && second != nullptr);

    // need to do a check to determine if the first object is the sphere or the second object is the sphere.

    auto* firstCollider = first->GetOwner()->GetComponent<CapsuleCollider>();
    auto* secondCollider = second->GetOwner()->GetComponent<CapsuleCollider>();

    // Ensuring both colliders exist
    assert(firstCollider && secondCollider);

    Vector3 normal = firstCollider->GetPosition() - secondCollider->GetClosestPoint(firstCollider->GetPosition());
    float dist_sq = normal.SquareLength();

    float combRad = firstCollider->m_radius + secondCollider->m_radius;

    // there is no collision.
    if (dist_sq >= combRad * combRad)
    {
        manifold->m_ContactCount = 0;
        return;
    }

    float dist = sqrtf(dist_sq);

    manifold->m_ContactCount = 1;

    if (dist == 0.f)
    {
        manifold->m_PenetrationDepth = firstCollider->m_radius;
        manifold->m_Normal = Vector3(1.f, 0.f, 0.f);
        manifold->m_Contacts[0] = firstCollider->GetPosition();
    }
    else
    {
        manifold->m_PenetrationDepth = combRad - dist;
        manifold->m_Normal = normal / dist;
        manifold->m_Contacts[0] = manifold->m_Normal * firstCollider->m_radius + firstCollider->GetPosition();
    }
}
