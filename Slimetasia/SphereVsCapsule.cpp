#include "SphereVsCapsule.h"

#include <cassert>

#include "CapsuleCollider.h"
#include "Manifold.h"
#include "RigidbodyComponent.h"
#include "SphereCollider.h"

void SphereVsCapsule::ComputeCollisionData(Manifold* manifold, RigidbodyComponent* first, RigidbodyComponent* second)
{
    // ensuring all parameters are valid.
    assert(manifold != nullptr && first != nullptr && second != nullptr);

    // need to do a check to determine if the first object is the sphere or the second object is the sphere.

    auto* Capsule = first->GetOwner()->GetComponent<CapsuleCollider>();
    auto* Sphere = second->GetOwner()->GetComponent<SphereCollider>();

    // checking to see if we got the order reversed.
    if (!Capsule || !Sphere)
    {
        Sphere = first->GetOwner()->GetComponent<SphereCollider>();
        Capsule = second->GetOwner()->GetComponent<CapsuleCollider>();
    }

    Vector3 normal = Sphere->GetPosition() - Capsule->GetClosestPoint(Sphere->GetPosition());
    float dist_sq = normal.SquareLength();

    float combRad = Sphere->m_radius + Capsule->m_radius;

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
        manifold->m_PenetrationDepth = Capsule->m_radius;
        manifold->m_Normal = Vector3(1.f, 0.f, 0.f);
        manifold->m_Contacts[0] = Capsule->GetPosition();
    }
    else
    {
        manifold->m_PenetrationDepth = combRad - dist;
        manifold->m_Normal = normal / dist;
        manifold->m_Contacts[0] = manifold->m_Normal * Capsule->m_radius + Capsule->GetPosition();
    }
}
