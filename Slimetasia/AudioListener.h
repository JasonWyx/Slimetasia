#pragma once
// ===========================================================================|
// INCLUDES                                                                   |
// ===========================================================================|
#include <Windows.h>

#include "Action.h"
#include "AppConsole.h"
#include "Application.h"
#include "CorePrerequisites.h"
#include "External Libraries\imgui\imgui.h"
#include "IComponent.h"
#include "Transform.h"

// ===========================================================================|
// FORWARD DECLARATIONS                                                       |
// ===========================================================================|
class AudioSystem;

// ===========================================================================|
// AUDIOEMITTER                                                               |
// ===========================================================================|
class AudioListener : public IComponent
{
    /// Friend ------------------------------------------------------------------
    friend class AudioSystem;

    /// Variables ---------------------------------------------------------------
private:
    bool isMain_;

    /// Functions ---------------------------------------------------------------
public:
    AudioListener(GameObject* parentObject);
    ~AudioListener();

    void MakeMain(bool state);
    bool IsMain() const;
    Vector3 GetPosition() const;
    Vector3 GetForwardVector() const;
    Vector3 GetUpwardVector() const;

    REFLECT()
};
