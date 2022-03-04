#include "SphereVsSphere.h"

#include <cassert>

#include "Manifold.h"
#include "RigidbodyComponent.h"
#include "SphereCollider.h"

void SphereVsSphere::ComputeCollisionData(Manifold* manifold, RigidbodyComponent* first, RigidbodyComponent* second)
{
    // ensuring all parameters are valid.
    assert(manifold != nullptr && first != nullptr && second != nullptr);

    auto* firstCollider = first->GetOwner()->GetComponent<SphereCollider>();
    auto* secondCollider = second->GetOwner()->GetComponent<SphereCollider>();

    // ensures both colliders are spheres.
    assert(firstCollider != nullptr && secondCollider != nullptr);

    Vector3 normal = secondCollider->GetPosition() - firstCollider->GetPosition();
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
        // standard normal if the spheres are exactly on top of each other.
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
