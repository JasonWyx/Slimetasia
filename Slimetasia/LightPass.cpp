#include "LightPass.h"

#include <random>

#include "Camera.h"
#include "DirectionalLight.h"
#include "Input.h"
#include "LightBase.h"
#include "PointLight.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "SpotLight.h"

LightPass::LightPass(iVector2 const& viewportSize)
    : m_ViewportSize(viewportSize)
    , m_LightShaderPointLight(ResourceManager::Instance().CreateResource<Shader>("LightShaderPoint"))
    , m_LightShaderSpotLight(ResourceManager::Instance().CreateResource<Shader>("LightShaderSpot"))
    , m_LightShaderDirectionalLight(ResourceManager::Instance().CreateResource<Shader>("LightShaderDirectional"))
    , m_LightShaderAmbientLight(ResourceManager::Instance().CreateResource<Shader>("LightShaderAmbient"))
    , m_LightShaderEmissiveLight(ResourceManager::Instance().CreateResource<Shader>("LightShaderEmissive"))
    , m_NullShader(ResourceManager::Instance().CreateResource<Shader>("NullShader"))
    , m_SimpleBlurShader(ResourceManager::Instance().CreateResource<Shader>("SimpleBlurShader"))
    , m_SSAOShader(ResourceManager::Instance().CreateResource<Shader>("SSAOShader"))
    , m_GausBlurShader(ResourceManager::Instance().CreateResource<Shader>("GausianBlurShader"))
    , m_PointLightVolume("PointLightVolumeMesh", "EngineResources/StencilCube.fbx")
    , m_SpotLightVolume("SpotLightVolumeMesh", "EngineResources/StencilCube.fbx")
{
    m_LightShaderPointLight->SetVertShaderFilePath("BasicTransform.vert");
    m_LightShaderPointLight->SetFragShaderFilePath("LightPoint.frag");
    m_LightShaderPointLight->Compile();

    m_LightShaderSpotLight->SetVertShaderFilePath("BasicTransform.vert");
    m_LightShaderSpotLight->SetFragShaderFilePath("LightSpot.frag");
    m_LightShaderSpotLight->Compile();

    m_LightShaderDirectionalLight->SetVertShaderFilePath("BasicTransform.vert");
    m_LightShaderDirectionalLight->SetFragShaderFilePath("LightDirectional.frag");
    m_LightShaderDirectionalLight->Compile();

    m_LightShaderAmbientLight->SetVertShaderFilePath("BasicTransform.vert");
    m_LightShaderAmbientLight->SetFragShaderFilePath("LightAmbient.frag");
    m_LightShaderAmbientLight->Compile();

    m_LightShaderEmissiveLight->SetVertShaderFilePath("BasicTransform.vert");
    m_LightShaderEmissiveLight->SetFragShaderFilePath("LightEmissive.frag");
    m_LightShaderEmissiveLight->Compile();

    m_NullShader->SetVertShaderFilePath("BasicTransform.vert");
    m_NullShader->SetFragShaderFilePath("BasicEmpty.frag");
    m_NullShader->Compile();

    m_SSAOShader->SetVertShaderFilePath("BasicTransform.vert");
    m_SSAOShader->SetFragShaderFilePath("SSAO.frag");
    m_SSAOShader->Compile();

    m_SimpleBlurShader->SetVertShaderFilePath("BasicTransform.vert");
    m_SimpleBlurShader->SetFragShaderFilePath("SimpleBlur.frag");
    m_SimpleBlurShader->Compile();

    m_GausBlurShader->SetVertShaderFilePath("BasicTransform.vert");
    m_GausBlurShader->SetFragShaderFilePath("GaussianBlur.frag");
    m_GausBlurShader->Compile();

    m_PointLightVolume.Load();

    Color4 whitePixel = Color4(1.0f);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_NoShadowTexture);
    glTextureStorage2D(m_NoShadowTexture, 1, GL_RGBA8, 1, 1);
    glTextureSubImage2D(m_NoShadowTexture, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, &whitePixel);

    glCreateVertexArrays(1, &m_ScreenQuadVAO);
    glCreateBuffers(1, &m_ScreenQuadVBO);

    glVertexArrayVertexBuffer(m_ScreenQuadVAO, 0, m_ScreenQuadVBO, 0, sizeof(float) * 5);

    glEnableVertexArrayAttrib(m_ScreenQuadVAO, 0);
    glVertexArrayAttribBinding(m_ScreenQuadVAO, 0, 0);
    glVertexArrayAttribFormat(m_ScreenQuadVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(m_ScreenQuadVAO, 1);
    glVertexArrayAttribBinding(m_ScreenQuadVAO, 1, 0);
    glVertexArrayAttribFormat(m_ScreenQuadVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3);

    GLfloat bufferData[] = {-1, -1, 0, 0, 0, 1, -1, 0, 1, 0, 1, 1, 0, 1, 1, -1, 1, 0, 0, 1};

    glNamedBufferStorage(m_ScreenQuadVBO, sizeof(bufferData), bufferData, 0);

    // Setup SSAO
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
    std::default_random_engine randomEngine;

    for (unsigned i = 0; i < 64; ++i)
    {
        Vector3 sample = Vector3(randomFloats(randomEngine) * 2.0f - 1.0f, randomFloats(randomEngine) * 2.0f - 1.0f, randomFloats(randomEngine));

        sample.Normalize();
        sample *= randomFloats(randomEngine);
        float scale = i / 64.0f;

        scale = Math::Lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        m_SSAOKernel.push_back(sample);
    }

    for (unsigned i = 0; i < 16; ++i)
    {
        m_SSAONoise.emplace_back(randomFloats(randomEngine) * 2.0f - 1.0f, randomFloats(randomEngine) * 2.0f - 1.0f, 0.0f);
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &m_SSAONoiseTexture);
    glTextureStorage2D(m_SSAONoiseTexture, 1, GL_RGB16F, 4, 4);
    glTextureSubImage2D(m_SSAONoiseTexture, 0, 0, 0, 4, 4, GL_RGB, GL_FLOAT, m_SSAONoise.data());
    glTextureParameteri(m_SSAONoiseTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_SSAONoiseTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_SSAONoiseTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_SSAONoiseTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glCreateFramebuffers(1, &m_SSAOFramebuffer);
    glNamedFramebufferDrawBuffer(m_SSAOFramebuffer, GL_COLOR_ATTACHMENT0);

    glCreateFramebuffers(1, &m_SSAOBlurFramebuffer);
    glNamedFramebufferDrawBuffer(m_SSAOBlurFramebuffer, GL_COLOR_ATTACHMENT0);

    glCreateFramebuffers(1, &m_EmissiveFramebuffer);
    glNamedFramebufferDrawBuffer(m_EmissiveFramebuffer, GL_COLOR_ATTACHMENT0);

    BuildRenderTargets();
}

LightPass::~LightPass()
{
    glDeleteTextures(1, &m_NoShadowTexture);
    glDeleteTextures(1, &m_SSAOBlurRenderTarget);
    glDeleteTextures(1, &m_SSAONoiseTexture);
    glDeleteTextures(1, &m_SSAORenderTarget);
    glDeleteTextures(1, &m_EmissiveFirstPass);
    glDeleteTextures(1, &m_EmissiveSecondPass);

    glDeleteFramebuffers(1, &m_SSAOFramebuffer);
    glDeleteFramebuffers(1, &m_SSAOBlurFramebuffer);
    glDeleteFramebuffers(1, &m_EmissiveFramebuffer);

    glDeleteVertexArrays(1, &m_ScreenQuadVAO);
    glDeleteBuffers(1, &m_ScreenQuadVBO);
}

void LightPass::Render(Camera& camera, RenderLayer const& renderLayer, const GBuffers& gbuffers, GLuint mainFramebuffer, bool isWaterPass)
{
    glBindVertexArray(m_ScreenQuadVAO);

    const GLuint normalTexture = gbuffers[(int)GBuffer::WorldNormal];
    const GLuint diffuseTexture = gbuffers[(int)GBuffer::Diffuse];
    const GLuint specularTexture = gbuffers[(int)GBuffer::Specular];
    const GLuint positionTexture = gbuffers[(int)GBuffer::WorldPosition];

    const std::vector<DirectionalLight*>& directionalLights = renderLayer.GetDirectionalLights();

    if (!isWaterPass && camera.IsSSAOEnabled())
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFramebuffer);
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        m_SSAOShader->Enable();
        {
            static const GLuint positionTextureLoc = glGetUniformLocation(m_SSAOShader->GetProgramHandle(), "gPositionTexture");
            static const GLuint normalTextureLoc = glGetUniformLocation(m_SSAOShader->GetProgramHandle(), "gNormalTexture");
            static const GLuint textureNoiseLoc = glGetUniformLocation(m_SSAOShader->GetProgramHandle(), "gTextureNoise");
            static const GLuint noiseScaleLoc = glGetUniformLocation(m_SSAOShader->GetProgramHandle(), "gNoiseScale");
            static const GLuint samplesLoc = glGetUniformLocation(m_SSAOShader->GetProgramHandle(), "gSamples");
            static const GLuint viewMatLoc = glGetUniformLocation(m_SSAOShader->GetProgramHandle(), "gView");
            static const GLuint projMatLoc = glGetUniformLocation(m_SSAOShader->GetProgramHandle(), "gProj");
            static const GLuint mvpLoc = glGetUniformLocation(m_SSAOShader->GetProgramHandle(), "gMVP");

            glUniform1i(positionTextureLoc, 0);
            glUniform1i(normalTextureLoc, 1);
            glUniform1i(textureNoiseLoc, 2);
            glUniform2fv(noiseScaleLoc, 1, ValuePtrFloat(Vector2((float)m_ViewportSize.x, (float)m_ViewportSize.y) / 4.0f));
            glUniform3fv(samplesLoc, 64, ValuePtrFloat(m_SSAOKernel[0]));
            glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, ValuePtrFloat(camera.GetViewTransform()));
            glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, ValuePtrFloat(camera.GetProjTransform()));
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(Matrix4()));

            glBindTextureUnit(0, gbuffers[(int)GBuffer::WorldPosition]);
            glBindTextureUnit(1, gbuffers[(int)GBuffer::WorldNormal]);
            glBindTextureUnit(2, m_SSAONoiseTexture);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOBlurFramebuffer);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        m_SimpleBlurShader->Enable();
        {
            static const GLuint ssaoTextureLoc = glGetUniformLocation(m_SimpleBlurShader->GetProgramHandle(), "gSSAOInput");
            static const GLuint mvpLoc = glGetUniformLocation(m_SimpleBlurShader->GetProgramHandle(), "gMVP");

            glUniform1i(ssaoTextureLoc, 0);
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(Matrix4()));

            glBindTextureUnit(0, m_SSAORenderTarget);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mainFramebuffer);

    if (Input::Instance().GetKeyDown(KEY_P))
    {
        m_LightShaderPointLight->Compile();
        m_LightShaderSpotLight->Compile();
        m_LightShaderDirectionalLight->Compile();
        m_LightShaderAmbientLight->Compile();
    }
    // Copy depth values from G-Buffer

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

#define USE_BOUNDING_VOLUMES

#ifdef USE_BOUNDING_VOLUMES
    glBindVertexArray(m_PointLightVolume.GetVAO());
#else
    glBindVertexArray(m_ScreenQuadVAO);
#endif

    // Point light pass
    m_LightShaderPointLight->Enable();

    const std::vector<PointLight*>& pointLights = culledPointLights.get();

    if (!pointLights.empty())
    {
        // Get uniform locations
        static const GLuint diffuseLoc = glGetUniformLocation(m_LightShaderPointLight->GetProgramHandle(), "gDiffuse");
        static const GLuint normalLoc = glGetUniformLocation(m_LightShaderPointLight->GetProgramHandle(), "gNormal");
        static const GLuint specularLoc = glGetUniformLocation(m_LightShaderPointLight->GetProgramHandle(), "gSpecular");
        static const GLuint positionLoc = glGetUniformLocation(m_LightShaderPointLight->GetProgramHandle(), "gPosition");
        static const GLuint shadowMapLoc = glGetUniformLocation(m_LightShaderPointLight->GetProgramHandle(), "gShadowMap");
        static const GLuint shadowDistLoc = glGetUniformLocation(m_LightShaderPointLight->GetProgramHandle(), "gShadowDistance");
        static const GLuint screenSizeLoc = glGetUniformLocation(m_LightShaderPointLight->GetProgramHandle(), "gScreenSize");

        static const GLuint mvpLoc = glGetUniformLocation(m_LightShaderPointLight->GetProgramHandle(), "gMVP");
        static const GLuint cameraPositionLoc = glGetUniformLocation(m_LightShaderPointLight->GetProgramHandle(), "gCameraPosition");
        static const GLuint lightAttenLoc = glGetUniformLocation(m_LightShaderPointLight->GetProgramHandle(), "gLightAtten");
        static const GLuint fogAttenLoc = glGetUniformLocation(m_LightShaderPointLight->GetProgramHandle(), "gFogAtten");
        static const GLuint fogColorLoc = glGetUniformLocation(m_LightShaderPointLight->GetProgramHandle(), "gFogColor");
        static const GLuint lightPositionLoc = glGetUniformLocation(m_LightShaderPointLight->GetProgramHandle(), "lightPosition");
        static const GLuint lightColorLoc = glGetUniformLocation(m_LightShaderPointLight->GetProgramHandle(), "lightColor");
        static const GLuint lightIntensityLoc = glGetUniformLocation(m_LightShaderPointLight->GetProgramHandle(), "lightIntensity");

        // Set global light properties
        glUniform2iv(screenSizeLoc, 1, ValuePtrInt(m_ViewportSize));
        glUniform3fv(cameraPositionLoc, 1, ValuePtrFloat(camera.GetTransform()->GetWorldPosition()));
        glUniform3fv(lightAttenLoc, 1, ValuePtrFloat(camera.GetLightAttenuation()));
        glUniform2fv(fogAttenLoc, 1, ValuePtrFloat(camera.GetFogAttenuation()));
        glUniform3fv(fogColorLoc, 1, ValuePtrFloat(camera.GetFogColor()));
        glUniform1i(positionLoc, 0);
        glUniform1i(normalLoc, 1);
        glUniform1i(diffuseLoc, 2);
        glUniform1i(specularLoc, 3);

        glBindTextureUnit(0, positionTexture);
        glBindTextureUnit(1, normalTexture);
        glBindTextureUnit(2, diffuseTexture);
        glBindTextureUnit(3, specularTexture);

        for (PointLight* const pointLight : pointLights)
        {
#ifdef USE_BOUNDING_VOLUMES
            // STENCIL PASS BEGIN
            // Disable draw buffer
            glDrawBuffer(GL_NONE);
            // Enable depth buffer but disable writing to it
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            // Clear stencil buffer
            glEnable(GL_STENCIL_TEST);
            // Enable and setup stencil test
            glStencilFunc(GL_ALWAYS, 0, 0);
            glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
            glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
            glStencilMask(0xFF);

            glClearStencil(0);
            glClear(GL_STENCIL_BUFFER_BIT);

            float lightRadius = pointLight->GetShadowDistance();

            Matrix4 mvp = camera.GetViewProjTransform() * Matrix4::Translate(pointLight->GetTransform()->m_WorldPosition) * Matrix4::Scale(lightRadius);

            // Activate stencil volume

            m_NullShader->Enable();
            {
                static const GLuint mvpNullLoc = glGetUniformLocation(m_NullShader->GetProgramHandle(), "gMVP");
                glUniformMatrix4fv(mvpNullLoc, 1, GL_FALSE, ValuePtrFloat(mvp));
            }
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_PointLightVolume.GetIndices().size()), GL_UNSIGNED_INT, 0);
            // STENCIL PASS END
            m_LightShaderPointLight->Enable();

            glDisable(GL_DEPTH_TEST);
#else
            Matrix4 mvp = Matrix4(1.0f);
#endif

            GLuint shadowMap = pointLight->GetShadowMapTexture();

            glBindTextureUnit(4, shadowMap);
            glUniform1i(shadowMapLoc, 4);
            glUniform1f(shadowDistLoc, pointLight->GetShadowDistance());

            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(mvp));
            glUniform3fv(lightPositionLoc, 1, ValuePtrFloat(pointLight->GetTransform()->GetWorldPosition()));
            glUniform3fv(lightColorLoc, 1, ValuePtrFloat(pointLight->GetLightColor()));
            glUniform1fv(lightIntensityLoc, 1, ValuePtrFloat(pointLight->GetIntensity()));

            // Enable blending
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE);

#ifdef USE_BOUNDING_VOLUMES
            // Restore draw buffer
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            // Enable face culling
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            // Set stencil function and disable depth test
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
            glStencilMask(0xFF);

            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_PointLightVolume.GetIndices().size()), GL_UNSIGNED_INT, 0);

            glDisable(GL_STENCIL_TEST);
            glDisable(GL_CULL_FACE);
#else
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
#endif
            glDisable(GL_BLEND);
        }
    }

#ifdef USE_BOUNDING_VOLUMES
    glBindVertexArray(m_PointLightVolume.GetVAO());
#else
    glBindVertexArray(m_ScreenQuadVAO);
#endif

    // Spot light pass
    m_LightShaderSpotLight->Enable();

    const std::vector<SpotLight*>& spotLights = culledSpotLights.get();
    // const std::vector<SpotLight*>& spotLights = renderLayer.GetSpotLights();

    if (!spotLights.empty())
    {
        // Get uniform locations
        static const GLuint diffuseLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "gDiffuse");
        static const GLuint normalLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "gNormal");
        static const GLuint specularLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "gSpecular");
        static const GLuint positionLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "gPosition");
        static const GLuint shadowMapLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "gShadowMap");
        static const GLuint lightSpaceMatLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "gLightSpaceTransform");
        static const GLuint screenSizeLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "gScreenSize");

        static const GLuint mvpLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "gMVP");
        static const GLuint cameraPositionLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "gCameraPosition");
        static const GLuint lightAttenLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "gLightAtten");
        static const GLuint fogAttenLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "gFogAtten");
        static const GLuint fogColorLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "gFogColor");
        static const GLuint lightDirectionLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "lightDirection");
        static const GLuint lightPositionLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "lightPosition");
        static const GLuint lightColorLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "lightColor");
        static const GLuint lightIntensityLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "lightIntensity");
        static const GLuint innerAngleLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "innerAngle");
        static const GLuint outerAngleLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "outerAngle");
        static const GLuint falloffAngleLoc = glGetUniformLocation(m_LightShaderSpotLight->GetProgramHandle(), "falloffAngle");

        glUniform2iv(screenSizeLoc, 1, ValuePtrInt(m_ViewportSize));
        glUniform3fv(cameraPositionLoc, 1, ValuePtrFloat(camera.GetTransform()->GetWorldPosition()));
        glUniform3fv(lightAttenLoc, 1, ValuePtrFloat(camera.GetLightAttenuation()));
        glUniform2fv(fogAttenLoc, 1, ValuePtrFloat(camera.GetFogAttenuation()));
        glUniform3fv(fogColorLoc, 1, ValuePtrFloat(camera.GetFogColor()));
        glUniform1i(positionLoc, 0);
        glUniform1i(normalLoc, 1);
        glUniform1i(diffuseLoc, 2);
        glUniform1i(specularLoc, 3);

        glBindTextureUnit(0, positionTexture);
        glBindTextureUnit(1, normalTexture);
        glBindTextureUnit(2, diffuseTexture);
        glBindTextureUnit(3, specularTexture);

        for (SpotLight* const spotLight : spotLights)
        {
#ifdef USE_BOUNDING_VOLUMES
            // STENCIL PASS BEGIN
            // Disable draw buffer
            glDrawBuffer(GL_NONE);

            // Enable depth buffer but disable writing to it
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);

            // Allow rasterization of both faces
            glDisable(GL_CULL_FACE);

            // Enable and setup stencil test
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 0, 0);
            glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
            glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
            glStencilMask(0xFF);
            // Clear stencil buffer
            glClearStencil(0);
            glClear(GL_STENCIL_BUFFER_BIT);

            float lightRadius = spotLight->GetShadowDistance();

            Matrix4 mvp = camera.GetViewProjTransform() * Matrix4::Translate(spotLight->GetTransform()->m_WorldPosition) * Matrix4::Scale(lightRadius);

            m_NullShader->Enable();
            {
                static const GLuint mvpNullLoc = glGetUniformLocation(m_NullShader->GetProgramHandle(), "gMVP");
                glUniformMatrix4fv(mvpNullLoc, 1, GL_FALSE, ValuePtrFloat(mvp));
            }

            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_PointLightVolume.GetIndices().size()), GL_UNSIGNED_INT, 0);

            // STENCIL PASS END
            m_LightShaderSpotLight->Enable();

            glDisable(GL_DEPTH_TEST);
#else
            Matrix4 mvp = Matrix4(1.0f);
#endif
            GLuint shadowMap = spotLight->IsCastShadows() ? spotLight->GetShadowMapTexture() : m_NoShadowTexture;

            glBindTextureUnit(4, shadowMap);
            glUniform1i(shadowMapLoc, 4);

            glBindTextureUnit(4, shadowMap);
            glUniform1i(shadowMapLoc, 4);
            glUniformMatrix4fv(lightSpaceMatLoc, 1, GL_FALSE, ValuePtrFloat(spotLight->GetLightViewProjMatricies().front()));

            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(mvp));
            glUniform3fv(lightDirectionLoc, 1, ValuePtrFloat(spotLight->GetDirection()));
            glUniform3fv(lightPositionLoc, 1, ValuePtrFloat(spotLight->GetTransform()->GetWorldPosition()));
            glUniform3fv(lightColorLoc, 1, ValuePtrFloat(spotLight->GetLightColor()));
            glUniform1fv(lightIntensityLoc, 1, ValuePtrFloat(spotLight->GetIntensity()));
            glUniform1fv(innerAngleLoc, 1, ValuePtrFloat(Math::ToRadians(spotLight->GetInnerAngle())));
            glUniform1fv(outerAngleLoc, 1, ValuePtrFloat(Math::ToRadians(spotLight->GetOuterAngle())));
            glUniform1fv(falloffAngleLoc, 1, ValuePtrFloat(spotLight->GetFalloffExponent()));

            // Enable blending
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE);

#ifdef USE_BOUNDING_VOLUMES
            // Restore draw buffer
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glDisable(GL_DEPTH_TEST);

            // Set stencil function and disable depth test
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
            glStencilMask(0xFF);

            // Enable face culling
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);

            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_PointLightVolume.GetIndices().size()), GL_UNSIGNED_INT, 0);

            glDisable(GL_CULL_FACE);
            glCullFace(GL_BACK);

            glDisable(GL_STENCIL_TEST);
#else
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
#endif
            glDisable(GL_BLEND);
        }
    }

    // Directional light pass
    m_LightShaderDirectionalLight->Enable();

    glBindVertexArray(m_ScreenQuadVAO);

    if (!directionalLights.empty())
    {
        // Get uniform locations
        static const GLuint diffuseLoc = glGetUniformLocation(m_LightShaderDirectionalLight->GetProgramHandle(), "gDiffuse");
        static const GLuint normalLoc = glGetUniformLocation(m_LightShaderDirectionalLight->GetProgramHandle(), "gNormal");
        static const GLuint specularLoc = glGetUniformLocation(m_LightShaderDirectionalLight->GetProgramHandle(), "gSpecular");
        static const GLuint positionLoc = glGetUniformLocation(m_LightShaderDirectionalLight->GetProgramHandle(), "gPosition");
        static const GLuint shadowMapLoc = glGetUniformLocation(m_LightShaderDirectionalLight->GetProgramHandle(), "gShadowMap");
        static const GLuint lightSpaceMatLoc = glGetUniformLocation(m_LightShaderDirectionalLight->GetProgramHandle(), "gLightSpaceTransform");
        static const GLuint screenSizeLoc = glGetUniformLocation(m_LightShaderDirectionalLight->GetProgramHandle(), "gScreenSize");

        static const GLuint mvpLoc = glGetUniformLocation(m_LightShaderDirectionalLight->GetProgramHandle(), "gMVP");
        static const GLuint cameraPositionLoc = glGetUniformLocation(m_LightShaderDirectionalLight->GetProgramHandle(), "gCameraPosition");
        static const GLuint lightAttenLoc = glGetUniformLocation(m_LightShaderDirectionalLight->GetProgramHandle(), "gLightAtten");
        static const GLuint fogAttenLoc = glGetUniformLocation(m_LightShaderDirectionalLight->GetProgramHandle(), "gFogAtten");
        static const GLuint fogColorLoc = glGetUniformLocation(m_LightShaderDirectionalLight->GetProgramHandle(), "gFogColor");
        static const GLuint lightDirectionLoc = glGetUniformLocation(m_LightShaderDirectionalLight->GetProgramHandle(), "lightDirection");
        static const GLuint lightColorLoc = glGetUniformLocation(m_LightShaderDirectionalLight->GetProgramHandle(), "lightColor");
        static const GLuint lightIntensityLoc = glGetUniformLocation(m_LightShaderDirectionalLight->GetProgramHandle(), "lightIntensity");

        glUniform2iv(screenSizeLoc, 1, ValuePtrInt(this->m_ViewportSize));
        glUniform3fv(cameraPositionLoc, 1, ValuePtrFloat(camera.GetTransform()->GetWorldPosition()));
        glUniform3fv(lightAttenLoc, 1, ValuePtrFloat(camera.GetLightAttenuation()));
        glUniform2fv(fogAttenLoc, 1, ValuePtrFloat(camera.GetFogAttenuation()));
        glUniform3fv(fogColorLoc, 1, ValuePtrFloat(camera.GetFogColor()));
        glUniform1i(positionLoc, 0);
        glUniform1i(normalLoc, 1);
        glUniform1i(diffuseLoc, 2);
        glUniform1i(specularLoc, 3);

        glBindTextureUnit(0, positionTexture);
        glBindTextureUnit(1, normalTexture);
        glBindTextureUnit(2, diffuseTexture);
        glBindTextureUnit(3, specularTexture);

        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);

        for (DirectionalLight* const directionalLight : directionalLights)
        {
            Matrix4 mvp = Matrix4(1.0f);
            m_LightShaderDirectionalLight->Enable();

            GLuint shadowMap = directionalLight->IsCastShadows() ? directionalLight->GetShadowMapTexture() : m_NoShadowTexture;

            glBindTextureUnit(4, shadowMap);
            glUniform1i(shadowMapLoc, 4);
            glUniformMatrix4fv(lightSpaceMatLoc, 1, GL_FALSE, ValuePtrFloat(directionalLight->GetLightViewProjMatricies().front()));

            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(mvp));
            glUniform3fv(lightDirectionLoc, 1, ValuePtrFloat(directionalLight->GetDirection()));
            glUniform3fv(lightColorLoc, 1, ValuePtrFloat(directionalLight->GetLightColor()));
            glUniform1fv(lightIntensityLoc, 1, ValuePtrFloat(directionalLight->GetIntensity()));

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }

        glDisable(GL_BLEND);
    }

    // Ambient light pass
    m_LightShaderAmbientLight->Enable();
    glBindVertexArray(m_ScreenQuadVAO);

    if (camera.GetAmbientColor().Length() != 0)
    {
        // Get uniform locations
        static const GLuint positionLoc = glGetUniformLocation(m_LightShaderAmbientLight->GetProgramHandle(), "gPosition");
        static const GLuint normalLoc = glGetUniformLocation(m_LightShaderAmbientLight->GetProgramHandle(), "gNormal");
        static const GLuint diffuseLoc = glGetUniformLocation(m_LightShaderAmbientLight->GetProgramHandle(), "gDiffuse");
        static const GLuint specularLoc = glGetUniformLocation(m_LightShaderAmbientLight->GetProgramHandle(), "gSpecular");
        static const GLuint ssaoLoc = glGetUniformLocation(m_LightShaderAmbientLight->GetProgramHandle(), "gSSAO");

        static const GLuint mvpLoc = glGetUniformLocation(m_LightShaderAmbientLight->GetProgramHandle(), "gMVP");
        static const GLuint cameraPositionLoc = glGetUniformLocation(m_LightShaderAmbientLight->GetProgramHandle(), "gCameraPosition");
        static const GLuint fogAttenLoc = glGetUniformLocation(m_LightShaderAmbientLight->GetProgramHandle(), "gFogAtten");
        static const GLuint fogColorLoc = glGetUniformLocation(m_LightShaderAmbientLight->GetProgramHandle(), "gFogColor");
        static const GLuint lightColorLoc = glGetUniformLocation(m_LightShaderAmbientLight->GetProgramHandle(), "gLightColor");

        Matrix4 mvp = Matrix4(1.0f);

        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(mvp));
        glUniform3fv(lightColorLoc, 1, ValuePtrFloat(camera.GetAmbientColor()));
        glUniform3fv(cameraPositionLoc, 1, ValuePtrFloat(camera.GetTransform()->GetWorldPosition()));
        glUniform2fv(fogAttenLoc, 1, ValuePtrFloat(camera.GetFogAttenuation()));
        glUniform3fv(fogColorLoc, 1, ValuePtrFloat(camera.GetFogColor()));
        glUniform1i(positionLoc, 0);
        glUniform1i(normalLoc, 1);
        glUniform1i(diffuseLoc, 2);
        glUniform1i(specularLoc, 3);
        glUniform1i(ssaoLoc, 4);

        glBindTextureUnit(0, positionTexture);
        glBindTextureUnit(1, normalTexture);
        glBindTextureUnit(2, diffuseTexture);
        glBindTextureUnit(3, specularTexture);
        glBindTextureUnit(4, (!isWaterPass && camera.IsSSAOEnabled()) ? m_SSAOBlurRenderTarget : m_NoShadowTexture);

        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisable(GL_BLEND);
    }

    // if (!isWaterPass)
    {
        if (m_GausBlurShader->Enable())
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_EmissiveFramebuffer);

            static const GLuint mvpLoc = glGetUniformLocation(m_GausBlurShader->GetProgramHandle(), "gMVP");
            static const GLuint emissiveTexLoc = glGetUniformLocation(m_GausBlurShader->GetProgramHandle(), "gTexture");
            static const GLuint screenSizeLoc = glGetUniformLocation(m_GausBlurShader->GetProgramHandle(), "gScreenSize");
            static const GLuint horizontalLoc = glGetUniformLocation(m_GausBlurShader->GetProgramHandle(), "gHorizontal");

            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(Matrix4()));
            glUniform1i(emissiveTexLoc, 0);
            glUniform2iv(screenSizeLoc, 1, ValuePtrInt(m_ViewportSize));

            // First pass - horizontal
            glNamedFramebufferTexture(m_EmissiveFramebuffer, GL_COLOR_ATTACHMENT0, m_EmissiveFirstPass, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            glUniform1i(horizontalLoc, 1);
            glBindTextureUnit(0, gbuffers[(int)GBuffer::Emissive]);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            // Second pass - vertical
            glNamedFramebufferTexture(m_EmissiveFramebuffer, GL_COLOR_ATTACHMENT0, m_EmissiveSecondPass, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            glUniform1i(horizontalLoc, 0);
            glBindTextureUnit(0, m_EmissiveFirstPass);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }

        if (m_LightShaderEmissiveLight->Enable())
        {
            static const GLuint mvpLoc = glGetUniformLocation(m_LightShaderEmissiveLight->GetProgramHandle(), "gMVP");
            static const GLuint emissiveTexLoc = glGetUniformLocation(m_LightShaderEmissiveLight->GetProgramHandle(), "uEmissiveTex");

            glBindFramebuffer(GL_FRAMEBUFFER, mainFramebuffer);

            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE);

            glUniform1i(emissiveTexLoc, 0);
            glBindTextureUnit(0, m_EmissiveSecondPass);

            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(Matrix4(1.0f)));

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            glDisable(GL_BLEND);
        }
    }
}

void LightPass::SetViewportSize(iVector2 const& viewportSize)
{
    if (m_ViewportSize != viewportSize)
    {
        m_ViewportSize = viewportSize;
        BuildRenderTargets();
    }
}

void LightPass::StartLightCulling(const Camera& camera, const RenderLayer& renderLayer)
{
    culledPointLights = Application::Instance().GetThreadPool().enqueue(RenderLayer::GetCulledPointLights, std::ref(renderLayer), std::ref(camera));
    culledSpotLights = Application::Instance().GetThreadPool().enqueue(RenderLayer::GetCulledSpotLights, std::ref(renderLayer), std::ref(camera));
}

void LightPass::BuildRenderTargets()
{
    glDeleteTextures(1, &m_SSAORenderTarget);
    glDeleteTextures(1, &m_SSAOBlurRenderTarget);
    glDeleteTextures(1, &m_EmissiveFirstPass);
    glDeleteTextures(1, &m_EmissiveSecondPass);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_SSAORenderTarget);
    glTextureStorage2D(m_SSAORenderTarget, 1, GL_R16F, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_SSAORenderTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_SSAORenderTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_SSAORenderTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_SSAORenderTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glNamedFramebufferTexture(m_SSAOFramebuffer, GL_COLOR_ATTACHMENT0, m_SSAORenderTarget, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_SSAOBlurRenderTarget);
    glTextureStorage2D(m_SSAOBlurRenderTarget, 1, GL_R16F, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_SSAOBlurRenderTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_SSAOBlurRenderTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_SSAOBlurRenderTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_SSAOBlurRenderTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glNamedFramebufferTexture(m_SSAOBlurFramebuffer, GL_COLOR_ATTACHMENT0, m_SSAOBlurRenderTarget, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_EmissiveFirstPass);
    glTextureStorage2D(m_EmissiveFirstPass, 1, GL_RGBA16F, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_EmissiveFirstPass, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_EmissiveFirstPass, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_EmissiveFirstPass, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_EmissiveFirstPass, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_EmissiveSecondPass);
    glTextureStorage2D(m_EmissiveSecondPass, 1, GL_RGBA16F, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_EmissiveSecondPass, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_EmissiveSecondPass, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_EmissiveSecondPass, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_EmissiveSecondPass, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
