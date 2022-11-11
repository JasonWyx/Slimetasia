#include "RigidbodyComponent.h"

#include "CollisionMesh_3D.h"
#include "GameObject.h"
#include "PhysicsSystem.h"

RigidbodyComponent::RigidbodyComponent(GameObject* parentObject)
    : IComponent { parentObject, "RigidbodyComponent" }
    ,
    // m_Active(true),
    m_LinearVelocity(0.f, 0.f, 0.f)
    , m_Acceleration(0.f, 0.f, 0.f)
    ,
    // m_Offset(0.f, 0.f, 0.f),
    // m_AngularVelocity(0.f, 0.f, 0.f),
    m_IsGhost(false)
    , m_Mass(1.f)
    ,
    // m_Drag(0.f),
    m_Bodytype(getEnumValueFromString(Bodytype, "DYNAMIC"))
    ,
    // m_FreezeRot(false),
    m_Restitution(0.f)
    , m_AffectedByGravity(false)
// m_IsAffectedByFriction(false)
// m_IsValid(true)
{
    // if (parentObject && parentObject->GetParentLayer() && parentObject->GetParentLayer()->GetParentScene())
    if (parentObject)
    {
        // performs a dependancy check and adds if the component specified is not found.
        m_OwnerObject->AddIfDoesntExist<Transform>();

        if (parentObject->GetParentLayer() && parentObject->GetParentLayer()->GetParentScene()) Register();
    }
}

RigidbodyComponent::~RigidbodyComponent()
{
    GetInstance(PhysicsSystem).DeregisterRigidbody(this);
    // m_IsValid = false;
}

void RigidbodyComponent::SetBodytype(const Bodytype& bodytype)
{
    if (m_Bodytype != bodytype)
    {
        m_Bodytype = bodytype;

        if (bodytype == eBodytype_STATIC)
        {
            m_LinearVelocity.Zero();
            m_Mass = 0.f;
            m_InverseMass = 0.f;
        }
    }
}

void RigidbodyComponent::SetBodytype(const int& bodytype)
{
    std::cout << "input bodytype = " << bodytype << "\n";
    if (m_Bodytype != static_cast<Bodytype>(bodytype))
    {
        m_Bodytype = static_cast<Bodytype>(bodytype);

        if (bodytype == eBodytype_STATIC)
        {
            m_LinearVelocity.Zero();
            m_Mass = 0.f;
            m_InverseMass = 0.f;
        }
    }
}

void RigidbodyComponent::SetMass(const float& new_mass)
{
    // throw if the mass is less than 0.
    assert(new_mass >= 0.f);

    m_Mass = new_mass;

    if (m_Mass)
        m_InverseMass = 1.f / m_Mass;
    else
        m_InverseMass = 0.f;
}

void RigidbodyComponent::SetPosition(const Vector3& newPos)
{
    assert(m_OwnerObject);

    m_OwnerObject->GetComponent<Transform>()->SetWorldPosition(newPos);

    AABB tmp(this);

    GetInstance(PhysicsSystem).UpdateAABBTree(this, tmp);
}

Vector3 RigidbodyComponent::GetPosition() const
{
    assert(m_OwnerObject);

    return m_OwnerObject->GetComponent<Transform>()->GetWorldPosition();
}

void RigidbodyComponent::GetBounds(Vector3& min, Vector3& max)
{
    Vector3 localMin(FLT_MAX), localMax(FLT_MIN);

    for (auto& elem : m_3DColliders)
    {
        Vector3 tmpMin, tmpMax;

        elem->ComputeBounds(tmpMin, tmpMax);
        localMin = tmpMin < localMin ? tmpMin : localMin;
        localMax = tmpMax > localMax ? tmpMax : localMax;
    }

    min = localMin;
    max = localMax;
}

void RigidbodyComponent::AddVelocity(const Vector3& vel)
{
    m_LinearVelocity += vel;
}

void RigidbodyComponent::SetYVelocity(const float& val)
{
    m_LinearVelocity.y = val;
}

void RigidbodyComponent::AddForce(const Vector3& force)
{
    // F = ma.
    // m_Acceleration += (force / m_Mass);
    m_Force += force;
}

void RigidbodyComponent::ApplyImpulse(const Vector3& impulse, const Vector3& contactVector, const Matrix3& tensor)
{
    if (m_Bodytype != eBodytype_DYNAMIC) return;

    m_LinearVelocity += m_InverseMass * impulse;
    // m_AngularVelocity += tensor * contactVector.Cross(impulse);
}

void RigidbodyComponent::IntegrateForces(const float& dt)
{
    if (m_InverseMass == 0.f) return;

    /*
  m_Acceleration += m_Force * m_InverseMass;

  if(m_AffectedByGravity)
      m_Acceleration += PhysicsSystem::s_PhyWorldSettings.m_Gravity;


  m_LinearVelocity += m_Acceleration * (dt / 2.f);*/

    if (m_AffectedByGravity)
        m_LinearVelocity += (m_Force * m_InverseMass + PhysicsSystem::s_PhyWorldSettings.m_Gravity) * dt * 0.5f;
    else
        m_LinearVelocity += (m_Force * m_InverseMass) * dt * 0.5f;

    // TODO : implement angular velocity.
    // m_AngularVelocity += m_Torque *
}

void RigidbodyComponent::IntegrateVelocity(const float& dt)
{
    if (m_InverseMass == 0.f) return;

    SetPosition(GetPosition() + m_LinearVelocity * dt);

    // TODO : implement angular velocity.
    // SetOrientation(GetOrientation() + m_AngularVelocity * dt);
    IntegrateForces(dt);
}

void RigidbodyComponent::Initialize()
{
    m_InverseMass = m_Mass ? 1.f / m_Mass : 0.f;
}

void RigidbodyComponent::EditorInitialize()
{
    GetInstance(PhysicsSystem).EditorInsertIntoDTree(this);
}

void RigidbodyComponent::RefreshColliders()
{
    m_3DColliders = m_OwnerObject->GetAllComponentWithThisBase<CollisionMesh_3D>();
}

void RigidbodyComponent::Register()
{
    GetInstance(PhysicsSystem).RegisterRigidbody(this);
}

REFLECT_INIT(RigidbodyComponent)
REFLECT_PARENT(IComponent)
// REFLECT_PROPERTY(m_Active)
REFLECT_PROPERTY(m_LinearVelocity)
REFLECT_PROPERTY(m_Acceleration)
// REFLECT_PROPERTY(m_Offset)
// REFLECT_PROPERTY(m_AngularVelocity)
REFLECT_PROPERTY(m_IsGhost)
// REFLECT_PROPERTY(m_CollidesWithStatic)
REFLECT_PROPERTY(m_Mass)
// REFLECT_PROPERTY(m_Drag)
REFLECT_PROPERTY(m_Bodytype)
// REFLECT_PROPERTY(m_FreezeRot)
REFLECT_PROPERTY(m_Restitution)
REFLECT_PROPERTY(m_AffectedByGravity)
// REFLECT_PROPERTY(m_IsSleeping)
REFLECT_PROPERTY(m_StaticFriction)
REFLECT_PROPERTY(m_DynamicFriction)
// REFLECT_PROPERTY(m_IsAffectedByFriction)
REFLECT_END()
