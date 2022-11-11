#pragma once
#include "CorePrerequisites.h"
#include "IComponent.h"
#include "Mesh.h"

enum class MeshRenderMode
{
    Normal = 0,
    Wireframe
};

class Renderable : public IComponent
{
    Mesh* m_Mesh;
    Color m_MeshColor;
    // ADD: Material
    // ADD: Animation (Skeletal)

public:

    Renderable(GameObject* parentObject);
    ~Renderable() = default;

    virtual void OnActive() override;
    virtual void OnInactive() override;

    Mesh* GetMesh() const;
    void SetMesh(Mesh* mesh);
    Color GetMeshColor() const;
    void SetMeshColor(Color const& color);
};