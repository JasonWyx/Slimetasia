#pragma once
#include <GL/glew.h>

#include <vector>

#include "CorePrerequisites.h"
#include "MathDefs.h"
#include "RenderLayer.h"

class Camera;
class PointLight;
class SpotLight;
class DirectionalLight;
class EmissiveLight;
class MeshRenderer;

class ShadowPass
{
private:
    GLuint m_Framebuffer;
    HShader m_ShadowDirectionalShader;
    HShader m_ShadowPointShader;

    static std::vector<Vector3> GetShadowFrustumPoints(Vector3 direction, Vector3 position, float aspectRatio, float fov, float n, float f, bool isOrtho);

public:
    ShadowPass();
    ~ShadowPass();

    void Render(RenderLayer const& renderLayer);
};