#pragma once
#include <GL/glew.h>

#include <vector>

#include "MathDefs.h"

class Camera;
class MeshRenderer;

struct GBufferID
{
    enum
    {
        Diffuse = 0,
        Specular,
        Normal,
        Position,
        Depth,
        Count
    };
};

class GBufferRenderPass
{
private:

    GLuint m_Framebuffer;
    GLuint m_GBuffers[GBufferID::Count];
    // HShader m_GBufferShader;

public:

    GBufferRenderPass();
    ~GBufferRenderPass();

    void Render(Camera& camera, std::vector<MeshRenderer*> const& meshList);
    void SetViewportSize(iVector2 size);
};