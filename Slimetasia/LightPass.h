#pragma once
#include <GL/glew.h>

#include "GeometryPass.h"
#include "MathDefs.h"
#include "RenderLayer.h"

class Camera;

class LightPass
{
public:
    LightPass(iVector2 const& viewportSize);
    ~LightPass();

    void Render(Camera& camera, RenderLayer const& renderLayer, const GBuffers& gbuffers, GLuint mainFramebuffer, bool isWaterPass = false);
    void SetViewportSize(iVector2 const& viewportSize);
    void StartLightCulling(const Camera& camera, const RenderLayer& renderLayer);

private:
    iVector2 m_ViewportSize;

    // Shaders
    HShader m_LightShaderPointLight;
    HShader m_LightShaderSpotLight;
    HShader m_LightShaderDirectionalLight;
    HShader m_LightShaderAmbientLight;
    HShader m_LightShaderEmissiveLight;
    HShader m_NullShader;
    HShader m_GausBlurShader;

    // Volume buffers
    GLuint m_ScreenQuadVAO;
    GLuint m_ScreenQuadVBO;

    Mesh m_PointLightVolume;
    Mesh m_SpotLightVolume;

    GLuint m_NoShadowTexture;

    HShader m_SimpleBlurShader;
    HShader m_SSAOShader;
    GLuint m_SSAOFramebuffer;
    GLuint m_SSAORenderTarget;
    GLuint m_SSAONoiseTexture;
    std::vector<Vector3> m_SSAOKernel;
    std::vector<Vector3> m_SSAONoise;

    GLuint m_SSAOBlurFramebuffer;
    GLuint m_SSAOBlurRenderTarget;

    GLuint m_EmissiveFramebuffer;
    GLuint m_EmissiveFirstPass;
    GLuint m_EmissiveSecondPass;

    std::future<std::vector<PointLight*>> culledPointLights;
    std::future<std::vector<SpotLight*>> culledSpotLights;

private:
    void BuildRenderTargets();
};
