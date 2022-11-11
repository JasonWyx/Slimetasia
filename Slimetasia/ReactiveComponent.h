#pragma once

#include "CorePrerequisites.h"
#include "GameObject.h"
#include "IComponent.h"
#include "Transform.h"

class ReactiveComponent : public IComponent
{
public:

    ReactiveComponent(GameObject* parentObject = nullptr);
    ~ReactiveComponent();

    // getters
    bool IsMouseClick() const { return m_mouseClick; }
    bool IsMouseHover() const { return m_mouseHover; }
    bool IsMouseDown() const { return m_mouseDown; }

    // setters
    void SetMouseClick(const bool& mouseclick) { m_mouseClick = mouseclick; }
    void SetMouseHover(const bool& mousehover) { m_mouseHover = mousehover; }
    void SetMouseDown(const bool& mousedown) { m_mouseDown = mousedown; }

    // funcs
    void UpdateReactive();
    void ForceReset();

    // static objects to reduce instances of data access.
    static Vector2 s_mousepos;
    static bool s_mousedown, s_mouseclick;

    REFLECT()
private:

    bool m_mouseClick = false, m_mouseHover = false, m_mouseDown = false;
};