#include "GeometryPass.h"

#include "Camera.h"
#include "MathDefs.h"
#include "MeshAnimator.h"
#include "MeshRenderer.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Shader.h"
#include "ThreadPool.h"

GeometryPass::GeometryPass(const iVector2& viewportSize)
    : RenderPass(viewportSize)
    , m_GeomShader(ResourceManager::Instance().CreateResource<Shader>("GeometryShader"))
    , m_TextShader(ResourceManager::Instance().CreateResource<Shader>("TextShader"))
    , m_VideoShader(ResourceManager::Instance().CreateResource<Shader>("VideoShader"))
    , m_Framebuffer(0)
    , m_GBuffers{0}
    , m_DSBuffer(0)
    , m_ClipPlane(0)
{
    m_GeomShader->SetVertShaderFilePath("GeometryPass.vert");
    m_GeomShader->SetFragShaderFilePath("GeometryPass.frag");
    m_GeomShader->Compile();

    m_TextShader->SetVertShaderFilePath("TextRender.vert");
    m_TextShader->SetFragShaderFilePath("TextRender.frag");
    m_TextShader->Compile();

    m_VideoShader->SetVertShaderFilePath("VideoShader.vert");
    m_VideoShader->SetFragShaderFilePath("VideoShader.frag");
    m_VideoShader->Compile();

    glCreateFramebuffers(1, &m_Framebuffer);

    BuildRenderTargets();

    GLuint drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4,
#ifdef EDITOR_ENABLED
                            GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6
#endif
    };

    glNamedFramebufferDrawBuffers(m_Framebuffer, sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers);

    glCreateVertexArrays(1, &m_VertexArray);
    glCreateBuffers(1, &m_VertexBuffer);

    glVertexArrayVertexBuffer(m_VertexArray, 0, m_VertexBuffer, 0, sizeof(float) * 5);

    glEnableVertexArrayAttrib(m_VertexArray, 0);
    glVertexArrayAttribBinding(m_VertexArray, 0, 0);
    glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(m_VertexArray, 1);
    glVertexArrayAttribBinding(m_VertexArray, 1, 0);
    glVertexArrayAttribFormat(m_VertexArray, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3);

    GLfloat bufferData[] = {-1, -1, 0, 0, 0, 1, -1, 0, 1, 0, 1, 1, 0, 1, 1, -1, 1, 0, 0, 1};

    glNamedBufferStorage(m_VertexBuffer, sizeof(bufferData), bufferData, 0);
}

GeometryPass::~GeometryPass()
{
    glDeleteTextures(1, &m_DSBuffer);
    glDeleteTextures((int)GBuffer::Count, m_GBuffers.data());
    glDeleteFramebuffers(1, &m_Framebuffer);
}

void GeometryPass::Render(Camera* camera, const RenderLayer& renderLayer)
{
    // const std::vector<MeshRenderer*>& meshList = camera->IsUICamera() ? renderLayer.GetMeshRenderers() : culledMeshRenderers.get();
    const std::vector<MeshRenderer*>& meshList = renderLayer.GetMeshRenderers();
    const std::vector<TextRenderer*>& textList = renderLayer.GetTextRenderers();
    const std::vector<VideoPlayer*>& videoList = renderLayer.GetVideoPlayers();
    const iVector2 vpSize = camera->GetViewportSize();
    const iVector2 vpOffset = camera->GetViewportOffset();

    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

    glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
    glViewport(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);

    glClearColor(0, 0, 0, 0);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_GeomShader->Enable();
    {
        static const GLuint diffuseTextureLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gDiffuseTexture");
        static const GLuint normalTextureLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gNormalTexture");
        static const GLuint specularTextureLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gSpecularTexture");
        static const GLuint specularShininessLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gSpecularShininess");
        static const GLuint emissiveEnabledLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gEmissiveEnabled");
        static const GLuint emissiveTextureLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gEmissiveTexture");
        static const GLuint emissiveColorLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gEmissiveColor");
        static const GLuint modelTransformLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gModelTransform");
        static const GLuint nodeTransformLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gNodeTransform");
        static const GLuint viewProjTransformLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gViewProjTransform");
        static const GLuint meshColorLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gMeshColor");
        static const GLuint pickingIdLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gPickingId");
        static const GLuint textureFlagsLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gTextureFlags");
        static const GLuint isAnimatedLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gIsAnimated");
        static const GLuint boneTransformLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gBoneTransforms");
        static const GLuint clipPlaneLoc = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gClipPlane");
        static const GLuint locTilingEnabled = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gTilingEnabled");
        static const GLuint locTilingAxis = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gTilingAxis");
        static const GLuint locTilingSize = glGetUniformLocation(m_GeomShader->GetProgramHandle(), "gTilingSize");

        glUniformMatrix4fv(viewProjTransformLoc, 1, GL_FALSE, ValuePtrFloat(camera->GetViewProjTransform()));

        // UI render
        if (camera->IsUICamera())
        {
            std::vector<MeshRenderer*> sortedRenderers = meshList;

            std::sort(sortedRenderers.begin(), sortedRenderers.end(), [](MeshRenderer* a, MeshRenderer* b) {
                Transform* transformA = a->GetTransform();
                Transform* transformB = b->GetTransform();
                return transformA->m_WorldPosition.z < transformB->m_WorldPosition.z;
            });

            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);

            for (MeshRenderer* renderer : sortedRenderers)
            {
                // Render mesh
                HMesh mesh = renderer->GetMesh();
                Color4 meshColor = renderer->GetMeshColor();
                MeshAnimator* meshAnimator = renderer->GetOwner()->GetComponent<MeshAnimator>();

                if (!mesh.Validate()) continue;

                // If bone animation is valid
                if (meshAnimator && meshAnimator->GetBoneTransforms().size())
                {
                    glUniform1i(isAnimatedLoc, 1);

                    std::vector<Matrix4> const& boneTransforms = meshAnimator->GetBoneTransforms();

                    glUniformMatrix4fv(boneTransformLoc, (GLsizei)boneTransforms.size(), GL_FALSE, ValuePtrFloat(*boneTransforms.data()));
                }
                else
                {
                    glUniform1i(isAnimatedLoc, 0);
                }

                glBindVertexArray(mesh->GetVAO());
                glUniform4fv(meshColorLoc, 1, ValuePtrFloat(meshColor));
                glUniform3fv(emissiveColorLoc, 1, ValuePtrFloat(renderer->GetEmissiveColor()));
                glUniform1i(emissiveEnabledLoc, renderer->IsEmissiveEnabled());
                glUniform1i(pickingIdLoc, renderer->GetOwner()->GetID());
                glUniformMatrix4fv(modelTransformLoc, 1, GL_FALSE, ValuePtrFloat(renderer->GetTransform()->GetWorldTransformMatrix()));
                glUniform4fv(clipPlaneLoc, 1, ValuePtrFloat(m_ClipPlane));

                // Toggle tiling options
                if (renderer->IsTilingEnabled())
                {
                    glUniform1i(locTilingEnabled, 1);
                    glUniform1ui(locTilingAxis, renderer->GetTilingAxis());
                    glUniform1f(locTilingSize, renderer->GetTilingSize());
                    glBindSampler(0, renderer->GetTextureSampler());
                    glBindSampler(1, renderer->GetTextureSampler());
                    glBindSampler(2, renderer->GetTextureSampler());
                    glBindSampler(3, renderer->GetTextureSampler());
                }
                else
                {
                    glUniform1i(locTilingEnabled, 0);
                    glBindSampler(0, 0);
                    glBindSampler(1, 0);
                    glBindSampler(2, 0);
                    glBindSampler(3, 0);
                }

                // Sub meshes in mesh
                std::vector<MeshEntry> const& entries = mesh->GetMeshEntries();

                GLuint diffuseTexture = renderer->GetDiffuseTexture().Validate() ? renderer->GetDiffuseTexture()->GetHandle() : GL_NONE;
                GLuint normalTexture = renderer->GetNormalTexture().Validate() ? renderer->GetNormalTexture()->GetHandle() : GL_NONE;
                GLuint specularTexture = renderer->GetSpecularTexture().Validate() ? renderer->GetSpecularTexture()->GetHandle() : GL_NONE;
                GLuint emissiveTexture = renderer->GetEmissiveTexture().Validate() ? renderer->GetEmissiveTexture()->GetHandle() : GL_NONE;
                GLuint textureFlags[] = {diffuseTexture, normalTexture, specularTexture, emissiveTexture};

                glUniform4uiv(textureFlagsLoc, 1, textureFlags);

                // Activate and load textures
                glBindTextureUnit(0, diffuseTexture);
                glBindTextureUnit(1, normalTexture);
                glBindTextureUnit(2, specularTexture);
                glBindTextureUnit(3, emissiveTexture);

                glUniform1i(diffuseTextureLoc, 0);
                glUniform1i(normalTextureLoc, 1);
                glUniform1i(specularTextureLoc, 2);
                glUniform1i(emissiveTextureLoc, 3);

                for (MeshEntry const& entry : entries)
                {
                    glUniformMatrix4fv(nodeTransformLoc, 1, GL_FALSE, ValuePtrFloat(entry.m_NodeTransform));
                    glDrawElementsBaseVertex(GL_TRIANGLES, entry.m_Size, GL_UNSIGNED_INT, (void*)(entry.m_BaseIndex * sizeof(GLuint)), entry.m_BaseVertex);
                }
            }

            glDisable(GL_BLEND);
        }
        else
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);

            for (MeshRenderer* renderer : meshList)
            {
                // Render mesh
                HMesh mesh = renderer->GetMesh();
                Color4 meshColor = renderer->GetMeshColor();
                MeshAnimator* meshAnimator = renderer->GetOwner()->GetComponent<MeshAnimator>();

                if (meshColor.a != 1.0f) continue;

                if (!mesh.Validate()) continue;

                // If bone animation is valid
                if (meshAnimator && meshAnimator->GetBoneTransforms().size())
                {
                    glUniform1i(isAnimatedLoc, 1);

                    std::vector<Matrix4> const& boneTransforms = meshAnimator->GetBoneTransforms();

                    glUniformMatrix4fv(boneTransformLoc, (GLsizei)boneTransforms.size(), GL_FALSE, ValuePtrFloat(*boneTransforms.data()));
                }
                else
                {
                    glUniform1i(isAnimatedLoc, 0);
                }

                glBindVertexArray(mesh->GetVAO());
                glUniform4fv(meshColorLoc, 1, ValuePtrFloat(meshColor));
                glUniform1i(pickingIdLoc, renderer->GetOwner()->GetID());
                glUniform3fv(emissiveColorLoc, 1, ValuePtrFloat(renderer->GetEmissiveColor()));
                glUniform1i(emissiveEnabledLoc, renderer->IsEmissiveEnabled());
                glUniformMatrix4fv(modelTransformLoc, 1, GL_FALSE, ValuePtrFloat(renderer->GetTransform()->GetWorldTransformMatrix()));
                glUniform4fv(clipPlaneLoc, 1, ValuePtrFloat(m_ClipPlane));

                // Toggle tiling options
                if (renderer->IsTilingEnabled())
                {
                    glUniform1i(locTilingEnabled, 1);
                    glUniform1ui(locTilingAxis, renderer->GetTilingAxis());
                    glUniform1f(locTilingSize, renderer->GetTilingSize());
                    glBindSampler(0, renderer->GetTextureSampler());
                    glBindSampler(1, renderer->GetTextureSampler());
                    glBindSampler(2, renderer->GetTextureSampler());
                    glBindSampler(3, renderer->GetTextureSampler());
                }
                else
                {
                    glUniform1i(locTilingEnabled, 0);
                    glBindSampler(0, 0);
                    glBindSampler(1, 0);
                    glBindSampler(2, 0);
                    glBindSampler(3, 0);
                }

                // Sub meshes in mesh
                std::vector<MeshEntry> const& entries = mesh->GetMeshEntries();

                GLuint diffuseTexture = renderer->GetDiffuseTexture().Validate() ? renderer->GetDiffuseTexture()->GetHandle() : GL_NONE;
                GLuint normalTexture = renderer->GetNormalTexture().Validate() ? renderer->GetNormalTexture()->GetHandle() : GL_NONE;
                GLuint specularTexture = renderer->GetSpecularTexture().Validate() ? renderer->GetSpecularTexture()->GetHandle() : GL_NONE;
                GLuint emissiveTexture = renderer->GetEmissiveTexture().Validate() ? renderer->GetEmissiveTexture()->GetHandle() : GL_NONE;
                GLuint textureFlags[] = {diffuseTexture, normalTexture, specularTexture, emissiveTexture};

                glUniform4uiv(textureFlagsLoc, 1, textureFlags);

                // Activate and load textures
                glBindTextureUnit(0, diffuseTexture);
                glBindTextureUnit(1, normalTexture);
                glBindTextureUnit(2, specularTexture);
                glBindTextureUnit(3, emissiveTexture);

                glUniform1i(diffuseTextureLoc, 0);
                glUniform1i(normalTextureLoc, 1);
                glUniform1i(specularTextureLoc, 2);
                glUniform1i(emissiveTextureLoc, 3);

                for (MeshEntry const& entry : entries)
                {
                    glUniformMatrix4fv(nodeTransformLoc, 1, GL_FALSE, ValuePtrFloat(entry.m_NodeTransform));
                    glDrawElementsBaseVertex(GL_TRIANGLES, entry.m_Size, GL_UNSIGNED_INT, (void*)(entry.m_BaseIndex * sizeof(GLuint)), entry.m_BaseVertex);
                }
            }

            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
        }
    }

    m_TextShader->Enable();
    {
        static const GLuint MTransformLoc = glGetUniformLocation(m_TextShader->GetProgramHandle(), "gMTransform");
        static const GLuint VPTransformLoc = glGetUniformLocation(m_TextShader->GetProgramHandle(), "gVPTransform");
        static const GLuint faceCameraLoc = glGetUniformLocation(m_TextShader->GetProgramHandle(), "gFaceCamera");
        static const GLuint fontAtlasLoc = glGetUniformLocation(m_TextShader->GetProgramHandle(), "gFontAtlas");
        static const GLuint fontColorLoc = glGetUniformLocation(m_TextShader->GetProgramHandle(), "gFontColor");
        static const GLuint sdfLoc = glGetUniformLocation(m_TextShader->GetProgramHandle(), "gSDF");
        static const GLuint worldNormalLoc = glGetUniformLocation(m_TextShader->GetProgramHandle(), "gWorldNormal");
        static const GLuint outlineEnabledLoc = glGetUniformLocation(m_TextShader->GetProgramHandle(), "gOutlineEnabled");
        static const GLuint outlineSDFLoc = glGetUniformLocation(m_TextShader->GetProgramHandle(), "gOutlineSDF");
        static const GLuint outlineColorLoc = glGetUniformLocation(m_TextShader->GetProgramHandle(), "gOutlineColor");
        static const GLuint objectIDLoc = glGetUniformLocation(m_TextShader->GetProgramHandle(), "gObjectID");

        std::vector<TextRenderer*> sortedRenderers = textList;

        if (camera->IsUICamera())
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            std::sort(sortedRenderers.begin(), sortedRenderers.end(), [](TextRenderer* a, TextRenderer* b) {
                Transform* transformA = a->m_Transform;
                Transform* transformB = b->m_Transform;
                return transformA->m_WorldPosition.z < transformB->m_WorldPosition.z;
            });
        }
        else
        {
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
        }

        for (TextRenderer* renderer : sortedRenderers)
        {
            if (!renderer->m_Font.Validate()) continue;

            glBindVertexArray(renderer->GetVertexArrayObject());

            Matrix4 lookAtMatrix;

            if (renderer->m_FaceCamera)
            {
                Vector3 forward = (camera->GetTransform()->GetWorldPosition() - renderer->m_Transform->m_WorldPosition).Normalize();
                Vector3 right = (Vector3(0.0f, 1.0f, 0.0f).Cross(forward)).Normalize();
                Vector3 up = (forward.Cross(right)).Normalize();

                lookAtMatrix.SetCol3(0, right);
                lookAtMatrix.SetCol3(1, up);
                lookAtMatrix.SetCol3(2, forward);
            }

            Matrix4 MTransform = renderer->m_FaceCamera ? Matrix4::Translate(renderer->m_Transform->m_WorldPosition) * lookAtMatrix * Matrix4::Scale(renderer->m_FontSize)
                                                        : renderer->m_Transform->GetWorldTransformMatrix();

            Matrix4 VPTransform = camera->GetViewProjTransform();

            Vector3 worldNormal = (MTransform * Vector4(0, 0, 1, 0)).V3();

            unsigned numIndices = renderer->GetTotalIndices();

            glUniformMatrix4fv(MTransformLoc, 1, GL_FALSE, ValuePtrFloat(MTransform));
            glUniformMatrix4fv(VPTransformLoc, 1, GL_FALSE, ValuePtrFloat(VPTransform));
            glUniform1i(faceCameraLoc, renderer->m_FaceCamera);
            glUniform1i(fontAtlasLoc, 0);
            glUniform4fv(fontColorLoc, 1, ValuePtrFloat(renderer->m_FontColor));
            glUniform1f(sdfLoc, 180.0f / 255.0f);
            glUniform3fv(worldNormalLoc, 1, ValuePtrFloat(worldNormal));

            glUniform1i(outlineEnabledLoc, renderer->m_OutlineEnabled);
            glUniform1f(outlineSDFLoc, (180.0f + renderer->m_OutlineWidth * (255.0f - 180.0f)) / 255.0f);
            glUniform4fv(outlineColorLoc, 1, ValuePtrFloat(renderer->m_OutlineColor));
            glUniform1i(objectIDLoc, renderer->GetOwner()->GetID());

            glBindTextureUnit(0, renderer->m_Font->GetTextureHandle());

            glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
        }

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }

    m_VideoShader->Enable();
    {
        static const GLuint MTransformLoc = glGetUniformLocation(m_VideoShader->GetProgramHandle(), "gMTransform");
        static const GLuint VPTransformLoc = glGetUniformLocation(m_VideoShader->GetProgramHandle(), "gVPTransform");
        static const GLuint frameTextureLoc = glGetUniformLocation(m_VideoShader->GetProgramHandle(), "gFrameTexture");
        static const GLuint objectIDLoc = glGetUniformLocation(m_VideoShader->GetProgramHandle(), "gObjectID");

        if (!camera->IsUICamera())
        {
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
        }
        else
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        for (VideoPlayer* renderer : videoList)
        {
            // Bind screen quad
            glBindVertexArray(m_VertexArray);

            if (renderer->m_FillScreen)
            {
                glDisable(GL_DEPTH_TEST);
            }
            else
            {
                glEnable(GL_DEPTH_TEST);
            }

            Matrix4 MTransform = renderer->m_FillScreen ? Matrix4(1.0f) : renderer->GetTransform()->GetWorldTransformMatrix() * Matrix4::Scale(Vector3(renderer->GetFrameRatio(), 1.0f, 1.0f));
            Matrix4 VPTransform = renderer->m_FillScreen ? Matrix4(1.0f) : camera->GetViewProjTransform();

            glUniformMatrix4fv(MTransformLoc, 1, GL_FALSE, ValuePtrFloat(MTransform));
            glUniformMatrix4fv(VPTransformLoc, 1, GL_FALSE, ValuePtrFloat(VPTransform));
            glUniform1i(frameTextureLoc, 0);
            glUniform1i(objectIDLoc, renderer->GetOwner()->GetID());

            glBindTextureUnit(0, renderer->GetVideoFrameTexture());

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
}

void GeometryPass::BuildRenderTargets()
{
    glDeleteTextures((int)GBuffer::Count, m_GBuffers.data());
    glDeleteTextures(1, &m_DSBuffer);

    glCreateTextures(GL_TEXTURE_2D, (int)GBuffer::Count, m_GBuffers.data());
    glCreateTextures(GL_TEXTURE_2D, 1, &m_DSBuffer);

    glTextureStorage2D(m_DSBuffer, 1, GL_DEPTH24_STENCIL8, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_DSBuffer, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_DSBuffer, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTextureStorage2D(m_GBuffers[(int)GBuffer::Diffuse], 1, GL_RGBA16F, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_GBuffers[(int)GBuffer::Diffuse], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_GBuffers[(int)GBuffer::Diffuse], GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTextureStorage2D(m_GBuffers[(int)GBuffer::Specular], 1, GL_RGBA16F, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_GBuffers[(int)GBuffer::Specular], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_GBuffers[(int)GBuffer::Specular], GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTextureStorage2D(m_GBuffers[(int)GBuffer::Emissive], 1, GL_RGB16F, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_GBuffers[(int)GBuffer::Emissive], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_GBuffers[(int)GBuffer::Emissive], GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTextureStorage2D(m_GBuffers[(int)GBuffer::WorldNormal], 1, GL_RGBA16F, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_GBuffers[(int)GBuffer::WorldNormal], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_GBuffers[(int)GBuffer::WorldNormal], GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTextureStorage2D(m_GBuffers[(int)GBuffer::WorldPosition], 1, GL_RGBA16F, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_GBuffers[(int)GBuffer::WorldPosition], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_GBuffers[(int)GBuffer::WorldPosition], GL_TEXTURE_MIN_FILTER, GL_NEAREST);

#ifdef EDITOR_ENABLED
    glTextureStorage2D(m_GBuffers[(int)GBuffer::TexCoords], 1, GL_RG16F, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_GBuffers[(int)GBuffer::TexCoords], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_GBuffers[(int)GBuffer::TexCoords], GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTextureStorage2D(m_GBuffers[(int)GBuffer::PickingID], 1, GL_R32I, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_GBuffers[(int)GBuffer::PickingID], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_GBuffers[(int)GBuffer::PickingID], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif

    glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, m_DSBuffer, 0);
    glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_GBuffers[(int)GBuffer::Diffuse], 0);
    glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT1, m_GBuffers[(int)GBuffer::Specular], 0);
    glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT2, m_GBuffers[(int)GBuffer::Emissive], 0);
    glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT3, m_GBuffers[(int)GBuffer::WorldNormal], 0);
    glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT4, m_GBuffers[(int)GBuffer::WorldPosition], 0);
#ifdef EDITOR_ENABLED
    glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT5, m_GBuffers[(int)GBuffer::TexCoords], 0);
    glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT6, m_GBuffers[(int)GBuffer::PickingID], 0);
#endif

    // Check framebuffer creation is a success
    // GLenum status = glCheckNamedFramebufferStatus(m_Framebuffer, GL_FRAMEBUFFER);
    // p_assert(status == GL_FRAMEBUFFER_COMPLETE);
}

const GBuffers& GeometryPass::GetGBuffers() const
{
    return m_GBuffers;
}

GLuint GeometryPass::GetFramebuffer() const
{
    return m_Framebuffer;
}

GLuint GeometryPass::GetDepthBuffer() const
{
    return m_DSBuffer;
}

GLint GeometryPass::GetPickedID(iVector2 mousePosition) const
{
#ifdef EDITOR_ENABLED
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_Framebuffer);
    glReadBuffer(GL_COLOR_ATTACHMENT6);

    GLint id = 0;
    glReadPixels(mousePosition.x, mousePosition.y, 1, 1, GL_RED_INTEGER, GL_INT, &id);
    GLenum error = glGetError();

    glReadBuffer(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return id;
#else
    return -1;
#endif
}

void GeometryPass::SetClipPlane(const Vector4& clipPlane)
{
    m_ClipPlane = clipPlane;
}

void GeometryPass::StartMeshCulling(const Camera& camera, const RenderLayer& renderLayer)
{
    culledMeshRenderers = Application::Instance().GetThreadPool().enqueue(RenderLayer::GetCulledMeshRenderers, std::ref(renderLayer), std::ref(camera));
}
