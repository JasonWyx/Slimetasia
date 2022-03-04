#pragma once
#include "CorePrerequisites.h"

class RigidbodyComponent;
class CollisionMesh_3D;

struct Manifold
{
    Manifold(RigidbodyComponent* first, RigidbodyComponent* second);

    ~Manifold() = default;

    void Solve();  // To generate contact info from the 2 rigidbodies.

    void Initialize();  // Precalculate for impulses.

    void ApplyImpulse();

    void PositionCorrection();  // to stop the jitters. Currently still primitive.

    void InfMassCorrection();  // if the mass is 0.

    void SetColliders();

    bool operator==(const Manifold& rhs);

    bool operator!=(const Manifold& rhs);

    RigidbodyComponent* m_FirstBody;

    RigidbodyComponent* m_SecondBody;

    std::vector<CollisionMesh_3D*> m_FirstColliders;

    std::vector<CollisionMesh_3D*> m_SecondColliders;

    float m_PenetrationDepth;

    Vector3 m_Normal;

    Vector3 m_Contacts[2];

    uint m_ContactCount;

    float m_ResultantRestitution;

    float m_ResultantDynamicFriction;

    float m_ResultantStaticFriction;
};