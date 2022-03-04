#include "ReactiveComponent.h"

#include "CollisionMesh_2D.h"
#include "PhysicsSystem.h"

Vector2 ReactiveComponent::s_mousepos = Vector2{};
bool ReactiveComponent::s_mouseclick = false;
bool ReactiveComponent::s_mousedown = false;

ReactiveComponent::ReactiveComponent(GameObject* parentObject)
    : IComponent(parentObject, "ReactiveComponent")
{
    if (parentObject)
    {
        parentObject->GetComponent<Transform>();
        parentObject->GetComponent<CollisionMesh_2D>();
        // GetInstance(PhysicsSystem).RegisterReactive(this);
    }
}

ReactiveComponent::~ReactiveComponent()
{
    /*if (m_OwnerObject)
    {
        GetInstance(PhysicsSystem).DeregisterReactive(this);
    }*/
}

void ReactiveComponent::UpdateReactive()
{
    auto collider = m_OwnerObject->GetComponent<CollisionMesh_2D>();

    if (collider == nullptr)
    {
        std::cout << "Assert was triggered in UpdateReactive function inside ReactiveComponent. ";
        std::cout << m_OwnerObject->GetName() << " does not have a 2D collider mesh component." << std::endl;
        assert(false);
    }

    if (collider->IsCollidingWithMouse(s_mousepos))
    {
        if (s_mousedown)
        {
            m_mouseHover = false;
            m_mouseClick = false;
            m_mouseDown = true;
        }
        else if (s_mouseclick)
        {
            m_mouseHover = false;
            m_mouseClick = true;
            m_mouseDown = false;
        }
        else
        {
            m_mouseHover = true;
            m_mouseClick = false;
            m_mouseDown = false;
        }
    }
}

void ReactiveComponent::ForceReset()
{
    m_mouseDown = m_mouseClick = m_mouseHover = false;
}

REFLECT_INIT(ReactiveComponent)
REFLECT_PARENT(IComponent)
REFLECT_END()
