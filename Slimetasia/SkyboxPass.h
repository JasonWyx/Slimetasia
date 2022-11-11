#pragma once
#include <GL/glew.h>

#include "Camera.h"
#include "CorePrerequisites.h"

class SkyboxPass
{
private:

    GLuint m_SkyboxVAO;
    GLuint m_SkyboxVBO;
    HShader m_SkyboxShader;

public:

    SkyboxPass();
    ~SkyboxPass();

    void Render(Camera& camera);
};