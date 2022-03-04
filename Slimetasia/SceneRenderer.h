#pragma once
#include <GL\glew.h>
#include <Windows.h>

#include "ISystem.h"

class MeshRenderer;

class SceneRenderer : public ISystem<SceneRenderer>
{
    friend class ISystem<SceneRenderer>;

    GLuint m_VAO;
    GLuint m_VBO;
    GLuint m_EBO;

    SceneRenderer();
    ~SceneRenderer();

    void RenderLayer();

public:
    void Update(float dt);
};