#pragma once
#include <string>

#include "Utility.h"

class PhysicsMaterial
{
public:

    PhysicsMaterial(const std::string& name = "", const float& restitution = 0.f, const float& rollingResist = 0.f, const float& frictionCoeff = 0.f);

    ~PhysicsMaterial() = default;

    // getters
    float GetRestitution() const { return m_Restitution; }

    float GetRollingResistance() const { return m_RollingResistance; }

    float GetFrictionCoefficient() const { return m_FrictionCoefficient; }

    std::string GetPhysicsMaterialName() const { return m_PhysicsMaterialName; }

    // setters

    // Restitution is a value [0,1]
    void SetRestitution(const float& newRest);

    // Rolling resistance is a value > 0
    void SetRollingResistance(const float& newResist);

    // Friction coefficient is a value > 0
    void SetFrictionCoefficient(const float& newCoeff);

    void SetPhysicsMaterialName(const std::string& newName) { m_PhysicsMaterialName = newName; }

    static uint s_PhysicsMaterialCount;

private:

    std::string m_PhysicsMaterialName;

    float m_Restitution;

    float m_RollingResistance;

    float m_FrictionCoefficient;
};
