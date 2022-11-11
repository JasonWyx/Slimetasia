#pragma once
#include "IComponent.h"
#include "MathDefs.h"
#include "PhysicsDefs.h"
#include "SmartEnums.h"

enum Bodytype;

class CollisionMesh_3D;

class RigidbodyComponent : public IComponent
{
public:

    RigidbodyComponent(GameObject* parentObject);
    ~RigidbodyComponent() override;

    // setters
    void SetGhost(const bool& ghost) { m_IsGhost = ghost; }
    void SetRestitution(const float& new_res) { m_Restitution = new_res; }
    void SetVelocity(const Vector3& new_vel) { m_LinearVelocity = new_vel; }
    void SetAcceleration(const Vector3& new_acc) { m_Acceleration = new_acc; }
    void SetOffset(const Vector3 offset) { m_Offset = offset; }
    void SetGravityEnabled(const bool& grav) { m_AffectedByGravity = grav; }
    void SetStaticFriction(const float& frict) { m_StaticFriction = frict; }
    void SetDynamicFriction(const float& frict) { m_DynamicFriction = frict; }
    void SetBodytype(const Bodytype& bodytype);
    void SetBodytype(const int& bodytype);
    void SetMass(const float& new_mass);
    void SetPosition(const Vector3& newPos);

    // getters
    bool GetGhost() const { return m_IsGhost; }
    float GetMass() const { return m_Mass; }
    Bodytype GetBodytype() const { return m_Bodytype; }
    int GetBodytypeInt() const { return static_cast<int>(m_Bodytype); }
    float GetRestitution() const { return m_Restitution; }
    Vector3 GetVelocity() const { return m_LinearVelocity; }
    Vector3 GetAcceleration() const { return m_Acceleration; }
    Vector3 GetOffset() const { return m_Offset; }
    bool GetGravityEnabled() const { return m_AffectedByGravity; }
    float GetStaticFriction() const { return m_StaticFriction; }
    float GetDynamicFriction() const { return m_DynamicFriction; }
    float GetInverseMass() const { return m_InverseMass; }
    Vector3 GetForce() const { return m_Force; }
    Vector3 GetPosition() const;

    void GetBounds(Vector3& min, Vector3& max);

    // funcs
    void AddVelocity(const Vector3& vel);
    void SetYVelocity(const float& val);
    void AddForce(const Vector3& force);
    void ApplyImpulse(const Vector3& impulse, const Vector3& contactVector, const Matrix3& tensor);

    void IntegrateForces(const float& dt);
    void IntegrateVelocity(const float& dt);
    void Initialize();
    void EditorInitialize();
    void RefreshColliders();

    void Register() override;

    friend struct Manifold;
    friend class PhysicsSystem;

    /*new stuff
     *
     *
     *
     */
    REFLECT()

private:

    Vector3 m_LinearVelocity;
    Vector3 m_Acceleration;
    Vector3 m_Offset;
    bool m_IsGhost;
    float m_Mass;
    float m_InverseMass;
    Bodytype m_Bodytype;
    float m_Restitution;
    bool m_AffectedByGravity;
    bool m_IsSleeping;
    bool m_IsValid;
    Vector3 m_Force;
    float m_StaticFriction;
    float m_DynamicFriction;

    std::vector<CollisionMesh_3D*> m_3DColliders;
};
