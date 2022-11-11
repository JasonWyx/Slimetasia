#include "Manifold.h"

#include "BaseDetectionAlgorithm.h"
#include "CollisionMesh_3D.h"
#include "PhysicsSystem.h"
#include "RigidbodyComponent.h"

Manifold::Manifold(RigidbodyComponent* first, RigidbodyComponent* second)
    : m_FirstBody { first }
    , m_SecondBody { second }
    , m_PenetrationDepth(0.f)
    , m_Normal(0.f, 0.f, 0.f)
    , m_ContactCount(0)
    , m_ResultantRestitution(0.f)
    , m_ResultantDynamicFriction(0.f)
    , m_ResultantStaticFriction(0.f)
{
    assert(first && second);

    m_FirstColliders = m_FirstBody->m_3DColliders;
    m_SecondColliders = m_SecondBody->m_3DColliders;
}

void Manifold::Solve()
{
    for (auto firstCol : m_FirstColliders)
    {
        for (auto secondCol : m_SecondColliders)
        {
            if (m_ContactCount > 0) break;

            if (m_FirstBody->m_Bodytype == eBodytype_STATIC && m_SecondBody->m_Bodytype == eBodytype_STATIC) return;

            PhysicsSystem::s_ColDispatch.SelectAlgorithm(firstCol->GetCollisionShapeType(), secondCol->GetCollisionShapeType())->ComputeCollisionData(this, m_FirstBody, m_SecondBody);
        }
    }
}

void Manifold::Initialize()
{
    // calculating the resultant restitution
    m_ResultantRestitution = m_FirstBody->GetRestitution() > m_SecondBody->GetRestitution() ? m_SecondBody->GetRestitution() : m_FirstBody->GetRestitution();

    // calculating the resultant static and dynamic friction.
    m_ResultantStaticFriction = sqrtf(m_FirstBody->GetStaticFriction() * m_SecondBody->GetStaticFriction());

    m_ResultantDynamicFriction = sqrtf(m_FirstBody->GetDynamicFriction() * m_SecondBody->GetDynamicFriction());

    for (auto i = 0u; i < m_ContactCount; i++)
    {
        Vector3 radA = m_Contacts[i] - m_FirstBody->GetPosition();
        Vector3 radB = m_Contacts[i] - m_SecondBody->GetPosition();

        // Vector3 radVelocity = m_SecondBody->GetVelocity() + m_SecondBody->GetAngularVelocity().Cross(radA) -
        //					   m_FirstBody->GetVelocity() + m_FirstBody->GetAngularVelocity().Cross(radB);
        //
        ////if only gravity is affecting the object, there shouldnt be any restitution affecting it.
        // if (radVelocity.SquareLength() < ((1.f / 60.f) * PhysicsSystem::s_PhyWorldSettings.m_Gravity).SquareLength() + EPSILON)
        //	m_ResultantRestitution = 0.f;
    }
}

void Manifold::ApplyImpulse()
{
    // check if both objects possess infinite mass.
    if ((m_FirstBody->m_InverseMass + m_SecondBody->m_InverseMass) == 0.f)
    {
        InfMassCorrection();
        return;
    }

    if (m_FirstBody->m_IsGhost || m_SecondBody->m_IsGhost) return;

    for (auto i = 0u; i < m_ContactCount; i++)
    {
        // Getting the radius from the center of mass to the contact point.
        Vector3 radA = m_Contacts[i] - m_FirstBody->GetPosition();

        Vector3 radB = m_Contacts[i] - m_SecondBody->GetPosition();

        // getting the relative velocity.
        Vector3 radVelocity = m_SecondBody->GetVelocity() /*+ m_SecondBody->GetAngularVelocity().Cross(radB)*/ - m_FirstBody->GetVelocity() /* + m_FirstBody->GetAngularVelocity().Cross(radA)*/;

        // getting the relative velocity along the normal.
        float contactVelocity = radVelocity.Dot(m_Normal);

        // no need for resolution if the velocities are separating.
        if (contactVelocity > 0.f) return;

        Vector3 radACrossNormal = radA.Cross(m_Normal);
        Matrix3 firstInertiaTensor { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
        // m_FirstCollider->ComputeInertiaTensor(firstInertiaTensor, m_FirstBody->m_Mass);
        Vector3 radBCrossNormal = radB.Cross(m_Normal);
        Matrix3 secondInertiaTensor { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
        // m_SecondCollider->ComputeInertiaTensor(secondInertiaTensor, m_SecondBody->m_Mass);

        float inverseMassSum = m_FirstBody->m_InverseMass + m_SecondBody->m_InverseMass + (firstInertiaTensor * radACrossNormal).Length() + (secondInertiaTensor * radBCrossNormal).Length();

        // calculating the impulse scalar.
        float j = -(1.f + m_ResultantRestitution) * contactVelocity;

        // only divide by the inverse mass sum IF BOTH bodies are dynamic!
        if (m_FirstBody->m_Bodytype == eBodytype_DYNAMIC && m_SecondBody->m_Bodytype == eBodytype_DYNAMIC) j /= inverseMassSum;

        j /= static_cast<float>(m_ContactCount);

        // apply the impulse
        Vector3 impulse = m_Normal * j;
        m_FirstBody->ApplyImpulse(-impulse, radA, firstInertiaTensor);
        m_SecondBody->ApplyImpulse(impulse, radB, secondInertiaTensor);

        // friction impulse
        radVelocity = m_SecondBody->GetVelocity() /* + m_SecondBody->GetAngularVelocity().Cross(radB) */ - m_FirstBody->GetVelocity() /* + m_FirstBody->GetAngularVelocity().Cross(radA)*/;

        Vector3 t = radVelocity - (m_Normal * radVelocity.Dot(m_Normal));
        t.Normalize();

        // j tangent magnitude
        float jt = -(radVelocity.Dot(t));

        // if (m_FirstBody->m_Bodytype == eBodytype_DYNAMIC && m_SecondBody->m_Bodytype == eBodytype_DYNAMIC)
        jt /= inverseMassSum;

        jt /= static_cast<float>(m_ContactCount);

        // if the friction impulse is too minute.
        if (jt >= -EPSILON && jt <= EPSILON) return;

        // coulumb's law
        Vector3 tangentImpulse;
        if (std::fabsf(jt) < j * m_ResultantStaticFriction)
            tangentImpulse = t * jt;
        else
            tangentImpulse = t * -j * m_ResultantDynamicFriction;

        // applying the friction impulse
        m_FirstBody->ApplyImpulse(-tangentImpulse, radA, firstInertiaTensor);
        m_SecondBody->ApplyImpulse(tangentImpulse, radB, secondInertiaTensor);
    }
}

void Manifold::PositionCorrection()
{
    if (m_FirstBody->m_IsGhost || m_SecondBody->m_IsGhost) return;

    const float k_slop = 0.01f;
    const float slop_percent = 0.4f;

    Vector3 correction = (std::max<float>(m_PenetrationDepth - k_slop, 0.f) / (m_FirstBody->m_InverseMass + m_SecondBody->m_InverseMass)) * m_Normal * slop_percent;

    if (m_FirstBody->m_Bodytype == eBodytype_DYNAMIC) m_FirstBody->SetPosition(m_FirstBody->GetPosition() - correction * m_FirstBody->GetInverseMass());

    if (m_SecondBody->m_Bodytype == eBodytype_DYNAMIC) m_SecondBody->SetPosition(m_SecondBody->GetPosition() + correction * m_SecondBody->GetInverseMass());
}

void Manifold::InfMassCorrection()
{
    m_FirstBody->m_LinearVelocity = Vector3 { 0.f, 0.f, 0.f };
    // m_FirstBody->m_AngularVelocity = Vector3{ 0.f, 0.f, 0.f };

    m_SecondBody->m_LinearVelocity = Vector3 { 0.f, 0.f, 0.f };
    // m_SecondBody->m_AngularVelocity = Vector3{ 0.f, 0.f, 0.f };
}

void Manifold::SetColliders()
{
    m_FirstColliders = m_FirstBody->m_3DColliders;
    m_SecondColliders = m_SecondBody->m_3DColliders;
}

bool Manifold::operator==(const Manifold& rhs)
{
    return m_FirstBody == rhs.m_FirstBody && m_SecondBody == rhs.m_SecondBody;
}

bool Manifold::operator!=(const Manifold& rhs)
{
    return !(*this == rhs);
}
