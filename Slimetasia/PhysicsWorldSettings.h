#pragma once
#include "CorePrerequisites.h"
#include "MemoryManager.h"
#include "ResourceManager.h"

// Position correction technique used in the contact solver (for contacts)
// BAUMGARTE_CONTACTS : Faster but can be innacurate and can lead to unexpected bounciness
//                      in some situations (due to error correction factor being added to
//                      the bodies momentum).
// SPLIT_IMPULSES : A bit slower but the error correction factor is not added to the
//                 bodies momentum. This is the option used by default.

struct PhysicsWorldSettings
{
    std::string m_WorldName = "";

    // distance threshold for 2 contact pts for a valid persistant contact.
    float m_PersistantContactDistThreshold = 0.03f;

    // default friction coefficient in the physics world.
    float m_DefaultFrictCoefficient = 0.003f;

    // default restitution for a rigidbody.
    float m_DefaultRestitution = 0.5f;

    // velocity threshold for a contact velocity resitution.
    float m_RestitutionVelThreshold = 1.0f;

    // default rolling resistance.
    float m_DefaultRollResis = 0.0f;

    // are bodies allowed to rest?
    bool m_IsSleepingEnabled = true;

    // duration that a body must stay still to be considered sleeping.
    float m_DefaultTimeBeforeSleep = 1.f;

    // any bodies with a linear vel less than this will enter sleep mode.
    float m_DefaultSleepLinearVel = 0.025f;

    // any bodies with an angular vel less than this will enter sleep mode.
    float m_DefaultSleepAngularVel = 3.0 * (PI) / 180.0f;

    // world gravity applied on all objects with the boolean turned on.
    Vector3 m_Gravity{0.f, -30.f, 0.f};

    // no. of iterations when solving the velocity constraints using the Sequential Impulse Technique.
    uint m_DefaultVelSolverIteration = 5;

    // no. of iterations when solving the position constraints using the Sequential Impulse Technique.
    uint m_DefaultPosSolverIteration = 5;

    // max no. of contact manifolds from 2 convex shapes.
    uint m_MaxConvexContactManifoldCount = 1;

    // max no. of contact manifolds from a collision involving at least 1 concave shape.
    uint m_MaxConcaveContactManifoldCount = 3;

    // If the cosine of the angle between the normals of 2 manifolds are greater than
    // the value specified, the manifolds are similar.
    float m_ContactManifoldSimilarAngle = 0.95f;

    void LoadFromFile(std::string filename = ResourceManager::s_PhysicsWorldSettingsDataDocument.string());

    void SaveToFile(std::string filename = ResourceManager::s_PhysicsWorldSettingsDataDocument.string()) const;
};