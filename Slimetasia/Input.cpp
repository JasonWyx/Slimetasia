#include "Input.h"

#include <stdio.h>

#include <algorithm>

#define TRANSITION_BIT 0x80  // 0 = pressed, 1 = released

// ============================================================================
// STATIC VARIABLES
// ============================================================================
ControllerMapping Input::controllerMap_;

// ============================================================================
// CONTROLLER MAPPING
// ============================================================================
int CKey::GetButtonUp()
{
    return Input::Instance().GetKeyUp(key_);
}
int CKey::GetButtonDown()
{
    return Input::Instance().GetKeyDown(key_);
}
int CKey::GetButtonPressed()
{
    return Input::Instance().GetKeyPressed(key_);
}
float CKey::GetAxis()
{
    return static_cast<float>(std::max(GetButtonPressed(), GetButtonDown()));
}
int CButton::GetButtonUp()
{
    return Input::Instance().GetButtonUp(key_);
}
int CButton::GetButtonDown()
{
    return Input::Instance().GetButtonDown(key_);
}
int CButton::GetButtonPressed()
{
    return Input::Instance().GetButtonPressed(key_);
}
float CButton::GetAxis()
{
    return static_cast<float>(std::max(GetButtonDown(), GetButtonPressed()));
}

int CStick::GetButtonUp()
{
    float tmp = GetAxis();
    return currAxis != 0.0f && tmp == 0.0f;
}
int CStick::GetButtonDown()
{
    return GetAxis() > 0.0f ? 1 : 0;
};
int CStick::GetButtonPressed()
{
    float tmp = GetAxis();
    return currAxis == 0 && tmp != 0;
}

float CStick::GetAxis()
{
    float tmp = 0;

    switch (key_)
    {
        case L_STICK:
            if (axis_[1] == 'x')
                tmp = Input::Instance().GetAxis(Input::Instance().GAMEPAD_LSTICK_X);
            else if (axis_[1] == 'y')
                tmp = Input::Instance().GetAxis(Input::Instance().GAMEPAD_LSTICK_Y);
            return (axis_[0] == '+') ? (tmp > 0.0f ? tmp : 0.0f) : (tmp < 0.0f ? -tmp : 0.0f);
            break;
        case R_STICK:
            if (axis_[1] == 'x')
                tmp = Input::Instance().GetAxis(Input::Instance().GAMEPAD_RSTICK_X);
            else if (axis_[1] == 'y')
                tmp = Input::Instance().GetAxis(Input::Instance().GAMEPAD_RSTICK_Y);
            return (axis_[0] == '+') ? (tmp > 0.0f ? tmp : 0.0f) : (tmp < 0.0f ? -tmp : 0.0f);
            break;
        case TRIGGER:
            if (axis_[0] == 'l') tmp = Input::Instance().GetTrigger(Input::Instance().GAMEPAD_LT);
            if (axis_[0] == 'r') tmp = Input::Instance().GetTrigger(Input::Instance().GAMEPAD_RT);
            break;
    }
    return tmp;
}
float CMouse::GetAxis()
{
    float val = 0.0f;

    if (axis_[1] == 'x')
        val = Input::Instance().GetMouseDelta().x;
    else if (axis_[1] == 'y')
        val = Input::Instance().GetMouseDelta().y;

    return (axis_[0] == '+') ? (val > 0.0f ? val : 0.0f) : (val < 0.0f ? -val : 0.0f);
}

ControllerMapping::ControllerMapping()
{
    SetControllerType(KEYBOARD);

    /// Load buttons from xml idealy <- ARIF ADD KEYS HERE
    /// If you want to use controls with axis change CButton to CStick and pass in enum STICK and axis (x/y) (l/r)

    // Movment
    kMapping_.insert(std::make_pair("Jump", std::make_shared<CKey>(KEY_SPACE)));
    kMapping_.insert(std::make_pair("Up", std::make_shared<CKey>(KEY_W)));
    kMapping_.insert(std::make_pair("Down", std::make_shared<CKey>(KEY_S)));
    kMapping_.insert(std::make_pair("Left", std::make_shared<CKey>(KEY_A)));
    kMapping_.insert(std::make_pair("Right", std::make_shared<CKey>(KEY_D)));
    kMapping_.insert(std::make_pair("Sprint", std::make_shared<CKey>(KEY_LSHIFT)));
    kMapping_.insert(std::make_pair("ReturnToLevelSelect", std::make_shared<CKey>(KEY_ESCAPE)));
    kMapping_.insert(std::make_pair("GetOut", std::make_shared<CKey>(KEY_ESCAPE)));

    cMapping_.insert(std::make_pair("Jump", std::make_shared<CButton>(GAMEPAD_A)));
    cMapping_.insert(std::make_pair("Up", std::make_shared<CStick>(CStick::L_STICK, "+y")));
    cMapping_.insert(std::make_pair("Down", std::make_shared<CStick>(CStick::L_STICK, "-y")));
    cMapping_.insert(std::make_pair("Left", std::make_shared<CStick>(CStick::L_STICK, "-x")));
    cMapping_.insert(std::make_pair("Right", std::make_shared<CStick>(CStick::L_STICK, "+x")));
    cMapping_.insert(std::make_pair("Sprint", std::make_shared<CButton>(GAMEPAD_LS)));
    cMapping_.insert(std::make_pair("ReturnToLevelSelect", std::make_shared<CButton>(GAMEPAD_START)));
    cMapping_.insert(std::make_pair("GetOut", std::make_shared<CButton>(GAMEPAD_B)));

    // Toggles
    kMapping_.insert(std::make_pair("SwitchMode", std::make_shared<CKey>(KEY_F)));
    cMapping_.insert(std::make_pair("SwitchMode", std::make_shared<CButton>(GAMEPAD_B)));

    // Shoot
    kMapping_.insert(std::make_pair("Shoot", std::make_shared<CKey>(MOUSE_LEFT)));
    cMapping_.insert(std::make_pair("Shoot", std::make_shared<CStick>(CStick::TRIGGER, "r")));

    // Start Wave
    kMapping_.insert(std::make_pair("StartWave", std::make_shared<CKey>(KEY_X)));
    cMapping_.insert(std::make_pair("StartWave", std::make_shared<CButton>(GAMEPAD_X)));

    // Show Hint
    kMapping_.insert(std::make_pair("ShowHint", std::make_shared<CKey>(KEY_TAB)));
    cMapping_.insert(std::make_pair("ShowHint", std::make_shared<CButton>(GAMEPAD_BACK)));

    // Trap
    kMapping_.insert(std::make_pair("SelectPrev", std::make_shared<CKey>(KEY_Q)));
    kMapping_.insert(std::make_pair("SelectNext", std::make_shared<CKey>(KEY_E)));
    kMapping_.insert(std::make_pair("Rotate", std::make_shared<CKey>(KEY_R)));

    cMapping_.insert(std::make_pair("SelectPrev", std::make_shared<CButton>(GAMEPAD_LB)));
    cMapping_.insert(std::make_pair("SelectNext", std::make_shared<CButton>(GAMEPAD_RB)));
    cMapping_.insert(std::make_pair("Rotate", std::make_shared<CStick>(CStick::TRIGGER, "l")));
    cMapping_.insert(std::make_pair("RotateLeft", std::make_shared<CButton>(GAMEPAD_DPAD_LEFT)));
    cMapping_.insert(std::make_pair("RotateRight", std::make_shared<CButton>(GAMEPAD_DPAD_RIGHT)));

    // Selection
    kMapping_.insert(std::make_pair("Select", std::make_shared<CKey>(KEY_RETURN)));

    cMapping_.insert(std::make_pair("Select", std::make_shared<CButton>(GAMEPAD_START)));

    // Mouse look
    kMapping_.insert(std::make_pair("LookLeft", std::make_shared<CMouse>("-x")));
    kMapping_.insert(std::make_pair("LookRight", std::make_shared<CMouse>("+x")));
    kMapping_.insert(std::make_pair("LookUp", std::make_shared<CMouse>("+y")));
    kMapping_.insert(std::make_pair("LookDown", std::make_shared<CMouse>("-y")));

    cMapping_.insert(std::make_pair("LookLeft", std::make_shared<CStick>(CStick::R_STICK, "-x")));
    cMapping_.insert(std::make_pair("LookRight", std::make_shared<CStick>(CStick::R_STICK, "+x")));
    cMapping_.insert(std::make_pair("LookUp", std::make_shared<CStick>(CStick::R_STICK, "+y")));
    cMapping_.insert(std::make_pair("LookDown", std::make_shared<CStick>(CStick::R_STICK, "-y")));

    // Merlion ulti
    kMapping_.insert(std::make_pair("Ultimate", std::make_shared<CKey>(MOUSE_RIGHT)));
    cMapping_.insert(std::make_pair("Ultimate", std::make_shared<CButton>(GAMEPAD_Y)));
}

void ControllerMapping::SetControllerType(CONTROLLERTYPE type)
{
    currentMapping_ = type;
}
int ControllerMapping::GetControllerType()
{
    return currentMapping_;
}

int ControllerMapping::GetInputUp(std::string action)
{
    return (currentMapping_ == KEYBOARD) ? KeyboardGetButtonUp(action) : ControllerGetButtonUp(action);
}
int ControllerMapping::GetInputDown(std::string action)
{
    return (currentMapping_ == KEYBOARD) ? KeyboardGetButtonDown(action) : ControllerGetButtonDown(action);
}
int ControllerMapping::GetInputPressed(std::string action)
{
    return (currentMapping_ == KEYBOARD) ? KeyboardGetButtonPressed(action) : ControllerGetButtonPressed(action);
}
float ControllerMapping::GetInputAxis(std::string action)
{
    return (currentMapping_ == KEYBOARD) ? KeyboardInputAxis(action) : ControllerInputAxis(action);
}

int ControllerMapping::KeyboardGetButtonUp(std::string action)
{
    auto mapping = kMapping_.find(action);
    return (mapping != kMapping_.end()) ? mapping->second->GetButtonUp() : 0;
}
int ControllerMapping::KeyboardGetButtonDown(std::string action)
{
    auto mapping = kMapping_.find(action);
    return (mapping != kMapping_.end()) ? mapping->second->GetButtonDown() : 0;
}
int ControllerMapping::KeyboardGetButtonPressed(std::string action)
{
    auto mapping = kMapping_.find(action);
    return (mapping != kMapping_.end()) ? mapping->second->GetButtonPressed() : 0;
}
float ControllerMapping::KeyboardInputAxis(std::string action)
{
    auto mapping = kMapping_.find(action);
    return (mapping != kMapping_.end()) ? mapping->second->GetAxis() : 0.0f;
}
int ControllerMapping::ControllerGetButtonUp(std::string action)
{
    auto mapping = cMapping_.find(action);
    return (mapping != cMapping_.end()) ? mapping->second->GetButtonUp() : 0;
}
int ControllerMapping::ControllerGetButtonDown(std::string action)
{
    auto mapping = cMapping_.find(action);
    return (mapping != cMapping_.end()) ? mapping->second->GetButtonDown() : 0;
}
int ControllerMapping::ControllerGetButtonPressed(std::string action)
{
    auto mapping = cMapping_.find(action);
    return (mapping != cMapping_.end()) ? mapping->second->GetButtonPressed() : 0;
}
float ControllerMapping::ControllerInputAxis(std::string action)
{
    auto mapping = cMapping_.find(action);
    return (mapping != cMapping_.end()) ? mapping->second->GetAxis() : 0.0f;
}

void ControllerMapping::UpdateAll()
{
    // for (auto & c : kMapping_)
    //  c.second->currAxis = c.second->GetAxis();
    for (auto& c : cMapping_)
        if (std::dynamic_pointer_cast<CStick>(c.second)) c.second->currAxis = c.second->GetAxis();
}
// ============================================================================
// INPUT
// ============================================================================
Input::Input()
    : CurrentState()
    , PreviousState()
    , CurrMousePos()
    , PrevMousePos()
    , DeltaMousePos()
    , PrevXInputState()
    , CurrXInputState()
    , XInputDeadZoneX(0.1f)
    , XInputDeadZoneY(0.1f)
    , CurrentControllerState(false)
    , PrevControllerState(false)
    , IsMouseWrapping(false)
{
    // Change controller mode to controller by default
    controllerMap_.SetControllerType(ControllerMapping::KEYBOARD);
}

void Input::Update()
{
    UpdateController();
    // Save previous input states
    memcpy(PreviousState, CurrentState, sizeof(CurrentState));

    // Poll current input states
    bool error = GetKeyboardState(CurrentState);

    if (IsMouseWrapping)
    {
        MouseWrap();
    }
    // Save mouse delta
    DeltaMousePos = CurrMousePos - PrevMousePos;
    PrevMousePos = CurrMousePos;

    // controller is connected on port 0
    PrevXInputState = CurrXInputState;
    XInputGetState(0, &CurrXInputState);
}

bool Input::GetKeyDown(short key)
{
    return (CurrentState[key] & PreviousState[key] & TRANSITION_BIT) != 0;
}

bool Input::GetKeyUp(short key)
{
    return !(CurrentState[key] & TRANSITION_BIT) && (PreviousState[key] & TRANSITION_BIT);
}

bool Input::GetKeyPressed(short key)
{
    return (CurrentState[key] & TRANSITION_BIT) && !(PreviousState[key] & TRANSITION_BIT);
}

Vector2 Input::GetMousePosition()
{
    return CurrMousePos;
}

Vector2 Input::GetMouseDelta()
{
    return DeltaMousePos;
}

void Input::SetMousePosition(unsigned x, unsigned y)
{
    CurrMousePos.x = static_cast<float>(x);
    CurrMousePos.y = static_cast<float>(y);
}

bool Input::GetButtonDown(WORD button)
{
    if (XInputGetState(0, &CurrXInputState) == ERROR_SUCCESS)
        return (CurrXInputState.Gamepad.wButtons & button) != 0;
    else
        return false;
}

bool Input::GetButtonPressed(WORD button)
{
    if (XInputGetState(0, &CurrXInputState) == ERROR_SUCCESS)
        return (CurrXInputState.Gamepad.wButtons & button) && !(PrevXInputState.Gamepad.wButtons & button);
    else
        return false;
}

bool Input::GetButtonUp(WORD button)
{
    if (XInputGetState(0, &CurrXInputState) == ERROR_SUCCESS)
        return !(CurrXInputState.Gamepad.wButtons & button) && (PrevXInputState.Gamepad.wButtons & button);
    else
        return false;
}

float Input::GetAxis(float axis)
{
    float normAxis = fmaxf(-1.0f, axis / 32767.0f);

    // Annoying
    // if (abs(normAxis) > XInputDeadZoneX)
    //  std::cout << abs(normAxis) << " : " << XInputDeadZoneX << std::endl;
    //
    return (abs(normAxis) < XInputDeadZoneX ? 0.0f : normAxis);
}

float Input::GetTrigger(float trig)
{
    return trig / 255.0f;
}

/*
LeftMotor and RightMotor works with 0.0f to 1.0f
*/
void Input::Rumble(float leftMotor, float rightMotor)
{
    XINPUT_VIBRATION VibrationState;
    ZeroMemory(&VibrationState, sizeof(XINPUT_VIBRATION));

    VibrationState.wLeftMotorSpeed = int(leftMotor * 65535.0f);
    VibrationState.wRightMotorSpeed = int(rightMotor * 65535.0f);
    XInputSetState(0, &VibrationState);
}

void Input::MouseWrap()
{
    static const unsigned x = GetSystemMetrics(SM_CXSCREEN);
    static const unsigned y = GetSystemMetrics(SM_CYSCREEN);
    POINT pos;
    GetCursorPos(&pos);
    unsigned cx = pos.x;
    unsigned cy = pos.y;
    if (!cx && !cy)
    {
        SetCursorPos(x - 2, y - 2);
        CurrMousePos = Vector2(static_cast<float>(x - 2), 1.f);
        PrevMousePos = Vector2(static_cast<float>(x - 1), 0.f);
    }
    else if (cx == 0)
    {
        SetCursorPos(x - 2, cy);
        CurrMousePos.x = static_cast<float>(x - 2);
        PrevMousePos.x = static_cast<float>(x - 1);
    }
    else if (cy == 0)
    {
        SetCursorPos(cx, y - 2);
        CurrMousePos.y = 1.f;
        PrevMousePos.y = 0.f;
    }
    else if (cx == (x - 1) && cy == (y - 1))
    {
        SetCursorPos(1, 1);
        CurrMousePos = Vector2(1.f, static_cast<float>(y - 2));
        PrevMousePos = Vector2(0.f, static_cast<float>(y - 1));
    }
    else if (cx == (x - 1))
    {
        SetCursorPos(1, cy);
        CurrMousePos.x = 1.f;
        PrevMousePos.x = 0.f;
    }
    else if (cy == (y - 1))
    {
        SetCursorPos(cx, 1);
        CurrMousePos.y = static_cast<float>(y - 2);
        PrevMousePos.y = static_cast<float>(y - 1);
    }
}

void Input::UpdateController()
{
    DWORD isController;  // Is it connected?

    _XINPUT_STATE ConState;  // all states of controller

    ZeroMemory(&ConState, sizeof(_XINPUT_STATE));

    isController = XInputGetState(0, &ConState);
    if (isController == ERROR_SUCCESS)
    {
        CurrentControllerState = true;
    }
    else
    {
        CurrentControllerState = false;
    }
    if (CurrentControllerState != PrevControllerState)
    {
        if (CurrentControllerState)
        {
            std::cout << "Controller is connected!" << std::endl;
            SetControllerMode(ControllerMapping::CONTROLLER);
        }
        else
        {
            std::cout << "Controller is disconnected!" << std::endl;
            SetControllerMode(ControllerMapping::KEYBOARD);
        }
    }
    PrevControllerState = CurrentControllerState;

    // Update states
    controllerMap_.UpdateAll();
}

void Input::ToggleMouseWrap(bool wrap)
{
    IsMouseWrapping = wrap;
}

int Input::IsControllerConnected()
{
    if (CurrentControllerState != PrevControllerState)
        if (CurrentControllerState)
            return 1;
        else
            return -1;
    return 0;
}

void Input::SetControllerMode(ControllerMapping::CONTROLLERTYPE type)
{
    controllerMap_.SetControllerType(type);
}
int Input::GetControllerMode()
{
    return controllerMap_.GetControllerType();
}
int Input::GetControllerInputUp(std::string action)
{
    return controllerMap_.GetInputUp(action);
}
int Input::GetControllerInputDown(std::string action)
{
    return controllerMap_.GetInputDown(action);
}
int Input::GetControllerInputPressed(std::string action)
{
    return controllerMap_.GetInputPressed(action);
}
float Input::GetControllerInputAxis(std::string action)
{
    return controllerMap_.GetInputAxis(action);
}
