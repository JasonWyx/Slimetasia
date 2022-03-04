#pragma once
#include "CollisionCallback.h"

// base event listener class to inherit from.
struct PhysicsEventListener
{
    PhysicsEventListener() = default;

    ~PhysicsEventListener() = default;

    // func is called when there is a new contact point between 2 bodies.
    // contains info about the contact.
    virtual void NewContactFound(const CollisionCallback::CollisionCallbackInfo& info) {}

    // called at the start of an internal tick of the simulation step in the physics world.
    virtual void BeginInternalTick() {}

    // called at the end of an internal tick of the simulation step in the physics world.
    virtual void EndInternalTick() {}
};
