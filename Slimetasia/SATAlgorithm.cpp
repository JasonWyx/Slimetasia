#include "SATAlgorithm.h"

#include "BoxCollider.h"
#include "Manifold.h"
#include "RigidbodyComponent.h"
#include "SphereCollider.h"

void SATAlgorithm::CheckCollidingConvexVsConvex(Manifold* manifold, RigidbodyComponent* first, RigidbodyComponent* second)
{
    // ensuring all parameters are valid.
    assert(manifold != nullptr && first != nullptr && second != nullptr);

    BoxCollider* firstCollider = first->GetParentObject()->GetComponent<BoxCollider>();
    BoxCollider* secondCollider = second->GetParentObject()->GetComponent<BoxCollider>();

    assert(firstCollider && secondCollider);
}
