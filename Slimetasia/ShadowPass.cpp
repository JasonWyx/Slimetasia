#include "ShadowPass.h"

#include "DirectionalLight.h"
#include "MeshAnimator.h"
#include "MeshRenderer.h"
#include "PointLight.h"
#include "ResourceManager.h"
#include "SpotLight.h"

std::vector<Vector3> ShadowPass::GetShadowFrustumPoints(Vector3 direction, Vector3 position, float aspectRatio, float fov, float n, float f, bool isOrtho)
{
    std::vector<Vector3> results(8);

    Vector3 cameraUp = direction.Normalized().y == 1.0f ? Vector3(0.0f, 0.0f, 1.0f) : Vector3(0.0f, 1.0f, 0.0f);

    Vector3 zAxis = direction;
    zAxis.Normalize();
    Vector3 xAxis = zAxis.Cross(cameraUp).Normalized();
    Vector3 yAxis = xAxis.Cross(zAxis).Normalized();

    // Near/far plane center points
    Vector3 camPosition = position;
    Vector3 nearCenter = camPosition + zAxis * n;
    Vector3 farCenter = camPosition + zAxis * f;

    // Get projected viewport extents on near/far planes
    float e = tanf(fov * 0.5f);
    float nearExtY = e * n;
    float nearExtX = nearExtY * (aspectRatio);
    float farExtY = isOrtho ? e * f : nearExtY;
    float farExtX = isOrtho ? farExtY * (aspectRatio) : nearExtX;

    // Points are just offset from the center points along camera basis
    // lbn, rbn, rtn, ltn, lbf, rbf, rtf, ltf
    results[0] = nearCenter - xAxis * nearExtX - yAxis * nearExtY;  // lbn
    results[1] = nearCenter + xAxis * nearExtX - yAxis * nearExtY;  // rbn
    results[2] = nearCenter + xAxis * nearExtX + yAxis * nearExtY;  // rtn
    results[3] = nearCenter - xAxis * nearExtX + yAxis * nearExtY;  // ltn
    results[4] = farCenter - xAxis * farExtX - yAxis * farExtY;     // lbf
    results[5] = farCenter + xAxis * farExtX - yAxis * farExtY;     // rbf
    results[6] = farCenter + xAxis * farExtX + yAxis * farExtY;     // rtf
    results[7] = farCenter - xAxis * farExtX + yAxis * farExtY;     // ltf

    return results;
}

ShadowPass::ShadowPass()
    : m_Framebuffer(0)
    , m_ShadowDirectionalShader(ResourceManager::Instance().CreateResource<Shader>("ShadowDirectionalShader"))
    , m_ShadowPointShader(ResourceManager::Instance().CreateResource<Shader>("ShadowPointShader"))
{
    m_ShadowDirectionalShader->SetVertShaderFilePath("ShadowTransform.vert");
    m_ShadowDirectionalShader->SetFragShaderFilePath("ShadowDirectional.frag");
    m_ShadowDirectionalShader->Compile();

    m_ShadowPointShader->SetVertShaderFilePath("ShadowTransform.vert");
    m_ShadowPointShader->SetFragShaderFilePath("ShadowPoint.frag");
    m_ShadowPointShader->SetGeomShaderFilePath("ShadowPoint.geom");
    m_ShadowPointShader->Compile();

    // Create empty framebuffer
    glCreateFramebuffers(1, &m_Framebuffer);
    glNamedFramebufferDrawBuffer(m_Framebuffer, GL_NONE);
    glNamedFramebufferReadBuffer(m_Framebuffer, GL_NONE);
}

ShadowPass::~ShadowPass() {}

void ShadowPass::Render(RenderLayer const& renderLayer)
{
    std::vector<MeshRenderer*> const& meshRenderers = renderLayer.GetMeshRenderers();

    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    //////////////////////////////////////////////////////////////////////////////
    // Point light shadow pass
    //////////////////////////////////////////////////////////////////////////////

    m_ShadowPointShader->Enable();
    {
        static const GLuint modelTransformLoc = glGetUniformLocation(m_ShadowPointShader->GetProgramHandle(), "gMTransform");
        static const GLuint lightPositionLoc = glGetUniformLocation(m_ShadowPointShader->GetProgramHandle(), "gLightPosition");
        static const GLuint shadowDistanceLoc = glGetUniformLocation(m_ShadowPointShader->GetProgramHandle(), "gShadowDistance");
        static const GLuint isAnimatedLoc = glGetUniformLocation(m_ShadowPointShader->GetProgramHandle(), "gIsAnimated");
        static const GLuint lightSpaceTransformLoc = glGetUniformLocation(m_ShadowPointShader->GetProgramHandle(), "gLightSpaceTransforms");
        static const GLuint boneTransformLoc = glGetUniformLocation(m_ShadowPointShader->GetProgramHandle(), "gBoneTransforms");
        static const GLuint nodeTransformLoc = glGetUniformLocation(m_ShadowPointShader->GetProgramHandle(), "gNodeTransform");

        const std::vector<PointLight*>& pointLights = renderLayer.GetPointLights();

        for (PointLight* pointLight : pointLights)
        {
            GLuint shadowMap = pointLight->GetShadowMapTexture();
            GLuint shadowRes = pointLight->GetShadowResolution();

            // Bind current cubemap texture to depth attachment
            glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_ATTACHMENT, shadowMap, 0);

            // Set viewport and clear
            glViewport(0, 0, shadowRes, shadowRes);
            glClear(GL_DEPTH_BUFFER_BIT);

            if (pointLight->IsCastShadows())
            {
                std::vector<Matrix4> const& vpTransforms = pointLight->GetLightViewProjMatricies();

                glUniform3fv(lightPositionLoc, 1, ValuePtrFloat(pointLight->GetTransform()->GetWorldPosition()));
                glUniform1f(shadowDistanceLoc, pointLight->GetShadowDistance());
                glUniformMatrix4fv(lightSpaceTransformLoc, static_cast<GLsizei>(vpTransforms.size()), GL_FALSE, ValuePtrFloat(vpTransforms[0]));

                for (MeshRenderer* meshRenderer : meshRenderers)
                {
                    HMesh mesh = meshRenderer->GetMesh();
                    if (mesh.Validate() && meshRenderer->IsCastShadow())
                    {
                        Matrix4 modelTransform = meshRenderer->GetTransform()->GetWorldTransformMatrix();
                        glUniformMatrix4fv(modelTransformLoc, 1, GL_FALSE, ValuePtrFloat(modelTransform));

                        std::vector<MeshEntry> const& meshEntries = mesh->GetMeshEntries();
                        glBindVertexArray(mesh->GetVAO());

                        MeshAnimator* meshAnimator = meshRenderer->GetOwner()->GetComponent<MeshAnimator>();

                        // Setup animation data
                        if (meshAnimator && meshAnimator->GetBoneTransforms().size())
                        {
                            glUniform1i(isAnimatedLoc, 1);
                            std::vector<Matrix4> const& boneTransforms = meshAnimator->GetBoneTransforms();
                            glUniformMatrix4fv(boneTransformLoc, static_cast<GLsizei>(boneTransforms.size()), GL_FALSE, ValuePtrFloat(boneTransforms[0]));
                        }
                        else
                        {
                            glUniform1i(isAnimatedLoc, 0);
                        }

                        // Draw objects
                        for (MeshEntry const& entry : meshEntries)
                        {
                            glUniformMatrix4fv(nodeTransformLoc, 1, GL_FALSE, ValuePtrFloat(entry.m_NodeTransform));
                            glDrawElementsBaseVertex(GL_TRIANGLES, entry.m_Size, GL_UNSIGNED_INT, (void*)(entry.m_BaseIndex * sizeof(GLuint)), entry.m_BaseVertex);
                        }
                    }
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    //  Spot light shadow pass
    //////////////////////////////////////////////////////////////////////////////

    m_ShadowDirectionalShader->Enable();
    {
        static const GLuint modelTransformLoc = glGetUniformLocation(m_ShadowDirectionalShader->GetProgramHandle(), "gMTransform");
        static const GLuint isAnimatedLoc = glGetUniformLocation(m_ShadowDirectionalShader->GetProgramHandle(), "gIsAnimated");
        static const GLuint boneTransformLoc = glGetUniformLocation(m_ShadowDirectionalShader->GetProgramHandle(), "gBoneTransforms");
        static const GLuint nodeTransformLoc = glGetUniformLocation(m_ShadowDirectionalShader->GetProgramHandle(), "gNodeTransform");
        static const GLuint diffuseTexLoc = glGetUniformLocation(m_ShadowDirectionalShader->GetProgramHandle(), "uDiffuseTexture");
        static const GLuint diffuseEnabledLoc = glGetUniformLocation(m_ShadowDirectionalShader->GetProgramHandle(), "uDiffuseEnabled");

        const std::vector<SpotLight*>& spotLights = renderLayer.GetSpotLights();

        for (SpotLight* spotLight : spotLights)
        {
            if (spotLight->IsCastShadows())
            {
                GLuint shadowMap = spotLight->GetShadowMapTexture();
                GLuint shadowRes = spotLight->GetShadowResolution();

                // Bind current cubemap texture to depth attachment
                glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_ATTACHMENT, shadowMap, 0);

                // Set viewport and clear
                glViewport(0, 0, shadowRes, shadowRes);
                glClear(GL_DEPTH_BUFFER_BIT);

                Matrix4 vpTransform = spotLight->GetLightViewProjMatricies().front();

                for (MeshRenderer* meshRenderer : meshRenderers)
                {
                    HMesh mesh = meshRenderer->GetMesh();
                    if (mesh.Validate() && meshRenderer->IsCastShadow())
                    {
                        MeshAnimator* meshAnimator = meshRenderer->GetOwner()->GetComponent<MeshAnimator>();
                        Matrix4 mvp = vpTransform * meshRenderer->GetTransform()->GetWorldTransformMatrix();
                        glUniformMatrix4fv(modelTransformLoc, 1, GL_FALSE, ValuePtrFloat(mvp));

                        // Setup animation data
                        if (meshAnimator && meshAnimator->GetBoneTransforms().size())
                        {
                            glUniform1i(isAnimatedLoc, 1);
                            std::vector<Matrix4> const& boneTransforms = meshAnimator->GetBoneTransforms();
                            glUniformMatrix4fv(boneTransformLoc, static_cast<GLsizei>(boneTransforms.size()), GL_FALSE, ValuePtrFloat(boneTransforms[0]));
                        }
                        else
                        {
                            glUniform1i(isAnimatedLoc, 0);
                        }

                        GLuint diffuseTexture = meshRenderer->GetDiffuseTexture().Validate() ? meshRenderer->GetDiffuseTexture()->GetHandle() : GL_NONE;

                        glBindTextureUnit(0, diffuseTexture);
                        glUniform1i(diffuseTexLoc, 0);
                        glUniform1i(diffuseEnabledLoc, diffuseTexture);

                        std::vector<MeshEntry> const& meshEntries = mesh->GetMeshEntries();
                        glBindVertexArray(mesh->GetVAO());

                        // Draw objects
                        for (MeshEntry const& entry : meshEntries)
                        {
                            glUniformMatrix4fv(nodeTransformLoc, 1, GL_FALSE, ValuePtrFloat(entry.m_NodeTransform));
                            glDrawElementsBaseVertex(GL_TRIANGLES, entry.m_Size, GL_UNSIGNED_INT, (void*)(entry.m_BaseIndex * sizeof(GLuint)), entry.m_BaseVertex);
                        }
                    }
                }
            }
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////
        //  Directional light shadow pass
        /////////////////////////////////////////////////////////////////////////////////////////////////

        const std::vector<DirectionalLight*>& directionalLights = renderLayer.GetDirectionalLights();

        for (DirectionalLight* directionalLight : directionalLights)
        {
            if (directionalLight->IsCastShadows())
            {
                GLuint shadowMap = directionalLight->GetShadowMapTexture();
                GLuint shadowRes = directionalLight->GetShadowResolution();

                // Bind current cubemap texture to depth attachment
                glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_ATTACHMENT, shadowMap, 0);

                // Set viewport and clear
                glViewport(0, 0, shadowRes, shadowRes);
                glClear(GL_DEPTH_BUFFER_BIT);

                Matrix4 vpTransform = directionalLight->GetLightViewProjMatricies().front();

                for (MeshRenderer* meshRenderer : meshRenderers)
                {
                    HMesh mesh = meshRenderer->GetMesh();

                    if (meshRenderer->GetMesh().Validate() && meshRenderer->IsCastShadow())
                    {
                        Matrix4 mvp = vpTransform * meshRenderer->GetTransform()->GetWorldTransformMatrix();
                        glUniformMatrix4fv(modelTransformLoc, 1, GL_FALSE, ValuePtrFloat(mvp));

                        std::vector<MeshEntry> const& meshEntries = mesh->GetMeshEntries();
                        glBindVertexArray(mesh->GetVAO());

                        MeshAnimator* meshAnimator = meshRenderer->GetOwner()->GetComponent<MeshAnimator>();

                        // Setup animation data
                        if (meshAnimator && meshAnimator->GetBoneTransforms().size())
                        {
                            glUniform1i(isAnimatedLoc, 1);
                            std::vector<Matrix4> const& boneTransforms = meshAnimator->GetBoneTransforms();
                            glUniformMatrix4fv(boneTransformLoc, static_cast<GLsizei>(boneTransforms.size()), GL_FALSE, ValuePtrFloat(boneTransforms[0]));
                        }
                        else
                        {
                            glUniform1i(isAnimatedLoc, 0);
                        }

                        GLuint diffuseTexture = meshRenderer->GetDiffuseTexture().Validate() ? meshRenderer->GetDiffuseTexture()->GetHandle() : GL_NONE;

                        glBindTextureUnit(0, diffuseTexture);
                        glUniform1i(diffuseTexLoc, 0);
                        glUniform1i(diffuseEnabledLoc, diffuseTexture);

                        // Draw objects
                        for (MeshEntry const& entry : meshEntries)
                        {
                            glUniformMatrix4fv(nodeTransformLoc, 1, GL_FALSE, ValuePtrFloat(entry.m_NodeTransform));
                            glDrawElementsBaseVertex(GL_TRIANGLES, entry.m_Size, GL_UNSIGNED_INT, (void*)(entry.m_BaseIndex * sizeof(GLuint)), entry.m_BaseVertex);
                        }
                    }
                }
            }
        }
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}
