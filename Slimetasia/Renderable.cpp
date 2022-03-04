#include "Renderable.h"

#include "GameObject.h"

Renderable::Renderable(GameObject* parentObject)
    : IComponent(parentObject, "Renderable")
{
}

void Renderable::OnActive()
{
    m_ParentObject->GetParentLayer();
}

void Renderable::OnInactive() {}

Mesh* Renderable::GetMesh() const
{
    return nullptr;
}

void Renderable::SetMesh(Mesh* mesh) {}

Color Renderable::GetMeshColor() const
{
    return Color();
}

void Renderable::SetMeshColor(Color const& color) {}
