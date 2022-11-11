#pragma once
#include <Windows.h>
#include <joystickapi.h>
#include <xinput.h>

#include <map>
#include <memory>

#include "ISystem.h"

#define MOUSE_LEFT VK_LBUTTON
#define MOUSE_RIGHT VK_RBUTTON
#define MOUSE_MID VK_MBUTTON
#define KEY_BACK VK_BACK
#define KEY_TAB VK_TAB
#define KEY_CLEAR VK_CLEAR
#define KEY_RETURN VK_RETURN
#define KEY_LSHIFT VK_LSHIFT
#define KEY_RSHIFT VK_RSHIFT
#define KEY_LCTRL VK_LCONTROL
#define KEY_RCTRL VK_RCONTROL
#define KEY_LALT VK_LMENU
#define KEY_RALT VK_RMENU
#define KEY_CAPS VK_CAPITAL
#define KEY_ESCAPE VK_ESCAPE
#define KEY_NEXT VK_NEXT
#define KEY_END VK_END
#define KEY_HOME VK_HOME
#define KEY_LEFT VK_LEFT
#define KEY_UP VK_UP
#define KEY_RIGHT VK_RIGHT
#define KEY_DOWN VK_DOWN
#define KEY_INSERT VK_INSERT
#define KEY_DELETE VK_DELETE
#define KEY_SPACE VK_SPACE
#define KEY_0 0x30
#define KEY_1 0x31
#define KEY_2 0x32
#define KEY_3 0x33
#define KEY_4 0x34
#define KEY_5 0x35
#define KEY_6 0x36
#define KEY_7 0x37
#define KEY_8 0x38
#define KEY_9 0x39
#define KEY_A 0x41
#define KEY_B 0x42
#define KEY_C 0x43
#define KEY_D 0x44
#define KEY_E 0x45
#define KEY_F 0x46
#define KEY_G 0x47
#define KEY_H 0x48
#define KEY_I 0x49
#define KEY_J 0x4A
#define KEY_K 0x4B
#define KEY_L 0x4C
#define KEY_M 0x4D
#define KEY_N 0x4E
#define KEY_O 0x4F
#define KEY_P 0x50
#define KEY_Q 0x51
#define KEY_R 0x52
#define KEY_S 0x53
#define KEY_T 0x54
#define KEY_U 0x55
#define KEY_V 0x56
#define KEY_W 0x57
#define KEY_X 0x58
#define KEY_Y 0x59
#define KEY_Z 0x5A
#define KEY_NUM_0 VK_NUMPAD0
#define KEY_NUM_1 VK_NUMPAD1
#define KEY_NUM_2 VK_NUMPAD2
#define KEY_NUM_3 VK_NUMPAD3
#define KEY_NUM_4 VK_NUMPAD4
#define KEY_NUM_5 VK_NUMPAD5
#define KEY_NUM_6 VK_NUMPAD6
#define KEY_NUM_7 VK_NUMPAD7
#define KEY_NUM_8 VK_NUMPAD8
#define KEY_NUM_9 VK_NUMPAD9
#define KEY_MUL VK_MULTIPLY
#define KEY_ADD VK_ADD
#define KEY_SEP VK_SEPARATOR
#define KEY_SUB VK_SUBTRACT
#define KEY_DOT VK_DECIMAL
#define KEY_DIV VK_DIVIDE
#define KEY_F1 VK_F1
#define KEY_F2 VK_F2
#define KEY_F3 VK_F3
#define KEY_F4 VK_F4
#define KEY_F5 VK_F5
#define KEY_F6 VK_F6
#define KEY_F7 VK_F7
#define KEY_F8 VK_F8
#define KEY_F9 VK_F9
#define KEY_F10 VK_F10
#define KEY_F11 VK_F11
#define KEY_F12 VK_F12

#define GAMEPAD_DPAD_UP XINPUT_GAMEPAD_DPAD_UP
#define GAMEPAD_DPAD_DOWN XINPUT_GAMEPAD_DPAD_DOWN
#define GAMEPAD_DPAD_LEFT XINPUT_GAMEPAD_DPAD_LEFT
#define GAMEPAD_DPAD_RIGHT XINPUT_GAMEPAD_DPAD_RIGHT
#define GAMEPAD_START XINPUT_GAMEPAD_START
#define GAMEPAD_BACK XINPUT_GAMEPAD_BACK
#define GAMEPAD_LS XINPUT_GAMEPAD_LEFT_THUMB
#define GAMEPAD_RS XINPUT_GAMEPAD_RIGHT_THUMB
#define GAMEPAD_LB XINPUT_GAMEPAD_LEFT_SHOULDER
#define GAMEPAD_RB XINPUT_GAMEPAD_RIGHT_SHOULDER
#define GAMEPAD_A XINPUT_GAMEPAD_A
#define GAMEPAD_B XINPUT_GAMEPAD_B
#define GAMEPAD_X XINPUT_GAMEPAD_X
#define GAMEPAD_Y XINPUT_GAMEPAD_Y

/// For GetAxis only
#define GAMEPAD_LSTICK_X CurrXInputState.Gamepad.sThumbLX
#define GAMEPAD_LSTICK_Y CurrXInputState.Gamepad.sThumbLY
#define GAMEPAD_RSTICK_X CurrXInputState.Gamepad.sThumbRX
#define GAMEPAD_RSTICK_Y CurrXInputState.Gamepad.sThumbRY

/// For GetTrigger only
#define GAMEPAD_LT CurrXInputState.Gamepad.bLeftTrigger
#define GAMEPAD_RT CurrXInputState.Gamepad.bRightTrigger

// ============================================================================
// CONTROLLER MAPPING
// ============================================================================
struct CBase
{
    const short key_;
    float currAxis;
    CBase(short key = 0)
        : key_(key)
        , currAxis(0)
    {
    }
    virtual int GetButtonUp() { return 0; };
    virtual int GetButtonDown() { return 0; };
    virtual int GetButtonPressed() { return 0; };
    virtual float GetAxis() { return 0; };
};

struct CButton : public CBase
{
    CButton(short key = 0)
        : CBase(key)
    {
    }

    int GetButtonUp() override;
    int GetButtonDown() override;
    int GetButtonPressed() override;
    float GetAxis() override;
};

struct CKey : public CBase
{
    CKey(short key = 0)
        : CBase(key)
    {
    }

    int GetButtonUp() override;
    int GetButtonDown() override;
    int GetButtonPressed() override;
    float GetAxis() override;
};

struct CStick : public CBase
{
    enum STICK
    {
        L_STICK = 0,
        R_STICK = 1,
        TRIGGER = 2
    };

    const STICK key_;
    const std::string axis_;

private:
public:

    CStick(STICK key, std::string axis)
        : key_(key)
        , axis_(axis)
    {
    }

    int GetButtonUp() override;
    int GetButtonDown() override;
    int GetButtonPressed() override;
    float GetAxis() override;
};

struct CMouse : public CBase
{
    const std::string axis_;

    CMouse(std::string axis)
        : axis_(axis)
    {
    }

    float GetAxis() override;
};

class ControllerMapping
{
public:

    enum CONTROLLERTYPE
    {
        KEYBOARD = 0,
        CONTROLLER = 1
    };

    // VARIABLES
    // --------------------------------------------------------------------------
private:

    CONTROLLERTYPE currentMapping_;

    /// All actions in the game
    std::map<std::string, std::shared_ptr<CBase>> kMapping_;
    std::map<std::string, std::shared_ptr<CBase>> cMapping_;

    // PRIVATE FUNCTIONS
    // --------------------------------------------------------------------------
private:

    void LoadKeyboardMapping() {}
    void LoadControllerMapping() {}
    void ChangeKeyboardMapping() {}
    void ChangeControllerMapping() {}
    void SaveCurrentMapping() {}
    int KeyboardGetButtonUp(std::string action);
    int KeyboardGetButtonDown(std::string action);
    int KeyboardGetButtonPressed(std::string action);
    float KeyboardInputAxis(std::string action);
    int ControllerGetButtonUp(std::string action);
    int ControllerGetButtonDown(std::string action);
    int ControllerGetButtonPressed(std::string action);
    float ControllerInputAxis(std::string action);

    // PUBLIC FUNCTIONS
    // --------------------------------------------------------------------------
public:

    ControllerMapping();

    void SetControllerType(CONTROLLERTYPE type);
    int GetControllerType();
    int GetInputDown(std::string action);
    int GetInputUp(std::string action);
    int GetInputPressed(std::string action);
    float GetInputAxis(std::string action);
    void UpdateAll();
};

// ============================================================================
// INPUT
// ============================================================================
class Input : public ISystem<Input>
{
public:

    Input();

    void Update();

    bool GetKeyDown(short key);
    bool GetKeyUp(short key);
    bool GetKeyPressed(short key);
    Vector2 GetMousePosition();
    Vector2 GetMouseDelta();

    void SetMousePosition(unsigned x, unsigned y);
    bool GetButtonDown(WORD button);
    bool GetButtonPressed(WORD button);
    bool GetButtonUp(WORD button);
    float GetAxis(float axis);
    float GetTrigger(float trig);
    void Rumble(float leftMotor, float rightMotor);
    void UpdateController();
    void ToggleMouseWrap(bool wrap);
    int IsControllerConnected();  // 1 connected, 0 no change, -1 disconnected

private:

    bool IsMouseWrapping;

    bool CurrentControllerState;
    bool PrevControllerState;

    BYTE CurrentState[256];
    BYTE PreviousState[256];
    Vector2 CurrMousePos;
    Vector2 PrevMousePos;
    Vector2 DeltaMousePos;

    XINPUT_STATE PrevXInputState;
    XINPUT_STATE CurrXInputState;
    const float XInputDeadZoneX = 0.2f;
    const float XInputDeadZoneY = 0.2f;

private:

    void MouseWrap();

    /// Controller mapping ------------------------------------------------------
private:

    friend ControllerMapping;
    friend CButton;
    friend CStick;
    static ControllerMapping controllerMap_;

public:

    void SetControllerMode(ControllerMapping::CONTROLLERTYPE type);
    int GetControllerMode();
    int GetControllerInputUp(std::string action);
    int GetControllerInputDown(std::string action);
    int GetControllerInputPressed(std::string action);
    float GetControllerInputAxis(std::string action);
};

#define GetMousePressed(button) GetKeyPressed(button)
#define GetMouseDown(button) GetKeyDown(button)
#define GetMouseUp(button) GetKeyUp(button)