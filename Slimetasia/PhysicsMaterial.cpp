#include "PhysicsMaterial.h"

#include <cassert>

uint PhysicsMaterial::s_PhysicsMaterialCount = 0;

PhysicsMaterial::PhysicsMaterial(const std::string& name, const float& restitution, const float& rollingResist, const float& frictionCoeff)
    : m_PhysicsMaterialName(name)
    , m_Restitution(restitution)
    , m_RollingResistance(rollingResist)
    , m_FrictionCoefficient(frictionCoeff)
{
    if (name.empty()) m_PhysicsMaterialName = "Physics_Material_" + s_PhysicsMaterialCount;

    s_PhysicsMaterialCount++;
}

void PhysicsMaterial::SetRestitution(const float& newRest)
{
    assert(newRest <= 1.f && newRest >= 0.f);

    m_Restitution = newRest;
}

void PhysicsMaterial::SetRollingResistance(const float& newResist)
{
    assert(newResist >= 0.f);

    m_RollingResistance = newResist;
}

void PhysicsMaterial::SetFrictionCoefficient(const float& newCoeff)
{
    assert(newCoeff >= 0.f);

    m_FrictionCoefficient = newCoeff;
}
