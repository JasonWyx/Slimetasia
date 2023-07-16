#ifndef USE_VULKAN

#include "DebugPass.h"

#include "Camera.h"
#include "CorePrerequisites.h"
#include "Editor.h"
#include "GameObject.h"
#include "Layer.h"
#include "MeshAnimator.h"
#include "MeshRenderer.h"
#include "ResourceManager.h"

static const std::vector<Vector3> sTranslateGizmoMesh = { Vector3(0.0f, 0.0f, 0.0f),  Vector3(1.0f, 0.0f, 0.0f),  Vector3(0.9f, 0.1f, 0.0f), Vector3(0.9f, 0.0f, 0.1f),
                                                          Vector3(0.9f, -0.1f, 0.0f), Vector3(0.9f, 0.0f, -0.1f), Vector3(0.9f, 0.1f, 0.0f) };

static const std::vector<Vector3> sTranslatePanGizmoMesh = { Vector3(0.0f, 0.0f, 0.0f), Vector3(0.2f, 0.0f, 0.0f), Vector3(0.2f, 0.2f, 0.0f), Vector3(0.0f, 0.2f, 0.0f) };

static const std::vector<Vector3> sScaleGizmoMesh = { Vector3(0.0f, 0.0f, 0.0f),   Vector3(1.0f, 0.0f, 0.0f),

                                                      Vector3(0.9f, 0.1f, 0.1f),   Vector3(0.9f, -0.1f, 0.1f),  Vector3(0.9f, -0.1f, 0.1f), Vector3(0.9f, -0.1f, -0.1f),
                                                      Vector3(0.9f, -0.1f, -0.1f), Vector3(0.9f, 0.1f, -0.1f),  Vector3(0.9f, 0.1f, -0.1f), Vector3(0.9f, 0.1f, 0.1f),

                                                      Vector3(1.1f, 0.1f, 0.1f),   Vector3(1.1f, -0.1f, 0.1f),  Vector3(1.1f, -0.1f, 0.1f), Vector3(1.1f, -0.1f, -0.1f),
                                                      Vector3(1.1f, -0.1f, -0.1f), Vector3(1.1f, 0.1f, -0.1f),  Vector3(1.1f, 0.1f, -0.1f), Vector3(1.1f, 0.1f, 0.1f),

                                                      Vector3(0.9f, 0.1f, 0.1f),   Vector3(1.1f, 0.1f, 0.1f),   Vector3(0.9f, -0.1f, 0.1f), Vector3(1.1f, -0.1f, 0.1f),
                                                      Vector3(0.9f, -0.1f, -0.1f), Vector3(1.1f, -0.1f, -0.1f), Vector3(0.9f, 0.1f, -0.1f), Vector3(1.1f, 0.1f, -0.1f) };

static std::vector<Vector3> sRotateGizmoMesh;

DebugPass::DebugPass(iVector2 const& viewportSize)
    : m_ViewportSize(viewportSize)
    , m_DebugLineShader(ResourceManager::Instance().CreateResource<Shader>("DebugLineShader"))
    , m_DebugGizmoShader(ResourceManager::Instance().CreateResource<Shader>("DebugGizmoShader"))
    , m_DebugOutlineStencilShader(ResourceManager::Instance().CreateResource<Shader>("DebugOutlineStencilShader"))
    , m_DebugOutlineShader(ResourceManager::Instance().CreateResource<Shader>("DebugOutlineShader"))
    , m_DebugVertexArray(0)
    , m_DebugVertexBuffer(0)
    , m_DebugDrawCommands()
    , m_DebugDrawVertices()
    , m_SelectionBoxGeometry()
    , m_SelectionBoxEnabled(false)
    , m_SelectedObjects()
    , m_GizmoMode(GizmoMode::Translate)
{
    m_DebugLineShader->SetVertShaderFilePath("DebugLine.vert");
    m_DebugLineShader->SetFragShaderFilePath("DebugLine.frag");
    m_DebugLineShader->Compile();

    m_DebugGizmoShader->SetVertShaderFilePath("DebugGizmos.vert");
    m_DebugGizmoShader->SetFragShaderFilePath("DebugGizmos.frag");
    m_DebugGizmoShader->Compile();

    m_DebugOutlineStencilShader->SetVertShaderFilePath("DebugOutline.vert");
    m_DebugOutlineStencilShader->SetFragShaderFilePath("BasicEmpty.frag");
    m_DebugOutlineStencilShader->Compile();

    m_DebugOutlineShader->SetVertShaderFilePath("DebugOutline.vert");
    m_DebugOutlineShader->SetFragShaderFilePath("DebugOutline.frag");
    m_DebugOutlineShader->Compile();

    glGenVertexArrays(1, &m_DebugVertexArray);
    glBindVertexArray(m_DebugVertexArray);
    glGenBuffers(1, &m_DebugVertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, m_DebugVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    float angle = 0.0f;
    float angleDelta = TWO_PI / 36;

    while (angle <= TWO_PI)
    {
        sRotateGizmoMesh.emplace_back(0.0f, sinf(angle), cosf(angle));
        angle += angleDelta;
    }
}

DebugPass::~DebugPass()
{
    glDeleteVertexArrays(1, &m_DebugVertexArray);
    glDeleteBuffers(1, &m_DebugVertexBuffer);
}

void DebugPass::Render(Camera& camera, GeometryPass& geomPass)
{
    // Use framebuffer from LightPass

    glEnable(GL_DEPTH_TEST);

    unsigned const layerId = camera.GetOwner()->GetParentLayer()->GetId();
    std::vector<DebugDrawCommand>& commandList = m_DebugDrawCommands[layerId];
    std::vector<Vector3>& geometryList = m_DebugDrawVertices[layerId];

    // Generate grid lines based on camera position
    Vector3 cameraPosition = camera.GetTransform()->GetWorldPosition();

    int lineCount = static_cast<int>(camera.GetFarPlane()) / 2;

    int xMin = static_cast<int>(floor(cameraPosition.x - lineCount));
    int xMax = static_cast<int>(ceil(cameraPosition.x + lineCount));
    int zMin = static_cast<int>(floor(cameraPosition.z - lineCount));
    int zMax = static_cast<int>(ceil(cameraPosition.z + lineCount));

    int xCount = xMax - xMin;
    int zCount = zMax - zMin;

    m_GridGeometry.resize((xCount + zCount) * 2);

    for (int i = xMin; i + 1 < xMax; ++i)
    {
        m_GridGeometry[(i - xMin) * 2].x = static_cast<float>(i);
        m_GridGeometry[(i - xMin) * 2].z = static_cast<float>(zMin);
        m_GridGeometry[(i - xMin) * 2 + 1].x = static_cast<float>(i);
        m_GridGeometry[(i - xMin) * 2 + 1].z = static_cast<float>(zMax);
    }

    for (int i = zMin; i + 1 < zMax; ++i)
    {
        m_GridGeometry[xCount * 2 + (i - zMin) * 2].x = static_cast<float>(xMin);
        m_GridGeometry[xCount * 2 + (i - zMin) * 2].z = static_cast<float>(i);
        m_GridGeometry[xCount * 2 + (i - zMin) * 2 + 1].x = static_cast<float>(xMax);
        m_GridGeometry[xCount * 2 + (i - zMin) * 2 + 1].z = static_cast<float>(i);
    }

    if (!m_DebugLineShader->Enable()) return;

    glBindVertexArray(m_DebugVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_DebugVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, geometryList.size() * sizeof(geometryList[0]), geometryList.data(), GL_DYNAMIC_DRAW);

    GLuint viewProjTransformLoc = glGetUniformLocation(m_DebugLineShader->GetProgramHandle(), "viewProjTransform");
    GLuint colorLoc = glGetUniformLocation(m_DebugLineShader->GetProgramHandle(), "color");
    glUniformMatrix4fv(viewProjTransformLoc, 1, GL_FALSE, ValuePtrFloat(camera.GetViewProjTransform()));

    // Draw debug geometry
    for (DebugDrawCommand const& command : commandList)
    {
        GLenum primitiveMode;
        switch (command.m_Type)
        {
            case DebugPrimitiveType::Points: primitiveMode = GL_POINTS; break;
            case DebugPrimitiveType::Lines: primitiveMode = GL_LINES; break;
            case DebugPrimitiveType::LineLoops: primitiveMode = GL_LINE_LOOP; break;
            case DebugPrimitiveType::Triangles: primitiveMode = GL_TRIANGLES; break;
            default: primitiveMode = GL_POINTS;
        }

        glUniform4fv(colorLoc, 1, (GLfloat*)&(command.m_Color));
        glDrawArrays(primitiveMode, command.m_BaseVertex, command.m_VertexCount);
    }

    // Draw grid geometry
    glBufferData(GL_ARRAY_BUFFER, m_GridGeometry.size() * sizeof(m_GridGeometry[0]), m_GridGeometry.data(), GL_DYNAMIC_DRAW);
    glUniform4fv(colorLoc, 1, ValuePtrFloat(Vector4(0.5f, 0.5f, 0.5f, 1.0f)));
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_GridGeometry.size()));

    m_DebugDrawCommands.clear();
    m_DebugDrawVertices.clear();

    if (!m_SelectedObjects.empty())
    {
        // Fetch selected game objects
        GameObjectList selectedObjects = camera.GetOwner()->GetParentLayer()->GetObjectsById(m_SelectedObjects);

        Matrix4 viewProjTransform = camera.GetViewProjTransform();

        ////////////////////////////
        //// Perform stencil pass
        ////////////////////////////
        if (m_DebugOutlineStencilShader->Enable())
        {
            static const GLuint mvpLoc = glGetUniformLocation(m_DebugOutlineStencilShader->GetProgramHandle(), "gMVPTransform");
            static const GLuint boneTransformsLoc = glGetUniformLocation(m_DebugOutlineStencilShader->GetProgramHandle(), "gBoneTransforms");
            static const GLuint nodeTransformLoc = glGetUniformLocation(m_DebugOutlineStencilShader->GetProgramHandle(), "gNodeTransform");
            static const GLuint isAnimatedLoc = glGetUniformLocation(m_DebugOutlineStencilShader->GetProgramHandle(), "gIsAnimated");

            glEnable(GL_DEPTH_TEST);

            glEnable(GL_STENCIL_TEST);
            glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilMask(0xFF);

            glClearStencil(0);
            glClear(GL_STENCIL_BUFFER_BIT);

            // glDisable(GL_CULL_FACE);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);

            for (GameObject* obj : selectedObjects)
            {
                MeshRenderer* meshRenderer = obj->GetComponent<MeshRenderer>();
                if (meshRenderer != nullptr)
                {
                    HMesh mesh = meshRenderer->GetMesh();
                    if (mesh.Validate())
                    {
                        Transform* transform = obj->GetComponent<Transform>();
                        MeshAnimator* meshAnimator = obj->GetComponent<MeshAnimator>();

                        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(viewProjTransform * transform->GetWorldTransformMatrix()));

                        if (meshAnimator && meshAnimator->GetBoneTransforms().size())
                        {
                            glUniform1i(isAnimatedLoc, 1);

                            std::vector<Matrix4> const& boneTransforms = meshAnimator->GetBoneTransforms();

                            glUniformMatrix4fv(boneTransformsLoc, (GLsizei)boneTransforms.size(), GL_FALSE, ValuePtrFloat(*boneTransforms.data()));
                        }
                        else
                        {
                            glUniform1i(isAnimatedLoc, 0);
                        }

                        glBindVertexArray(mesh->GetVAO());

                        // Sub meshes in mesh
                        std::vector<MeshEntry> const& entries = mesh->GetMeshEntries();

                        for (MeshEntry const& entry : entries)
                        {
                            // glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(viewProjTransform * transform->GetWorldTransformMatrix() * entry.m_MeshTransform));
                            glUniformMatrix4fv(nodeTransformLoc, 1, GL_FALSE, ValuePtrFloat(entry.m_NodeTransform));

                            glDrawElementsBaseVertex(GL_TRIANGLES, entry.m_Size, GL_UNSIGNED_INT, (void*)(entry.m_BaseIndex * sizeof(GLuint)), entry.m_BaseVertex);
                        }
                    }
                }
            }

            glDisable(GL_DEPTH_TEST);
        }
        //////////////////////////
        // Perform outline pass
        //////////////////////////

        if (m_DebugOutlineShader->Enable())
        {
            static const GLuint mvpLoc = glGetUniformLocation(m_DebugOutlineShader->GetProgramHandle(), "gMVPTransform");
            static const GLuint boneTransformsLoc = glGetUniformLocation(m_DebugOutlineShader->GetProgramHandle(), "gBoneTransforms");
            static const GLuint nodeTransformLoc = glGetUniformLocation(m_DebugOutlineShader->GetProgramHandle(), "gNodeTransform");
            static const GLuint isAnimatedLoc = glGetUniformLocation(m_DebugOutlineShader->GetProgramHandle(), "gIsAnimated");

            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glStencilFunc(GL_EQUAL, 0, 0xFF);
            glStencilMask(0x00);

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(5);

            for (GameObject* obj : selectedObjects)
            {
                MeshRenderer* meshRenderer = obj->GetComponent<MeshRenderer>();
                if (meshRenderer != nullptr)
                {
                    HMesh mesh = meshRenderer->GetMesh();
                    if (mesh.Validate())
                    {
                        Transform* transform = obj->GetComponent<Transform>();
                        MeshAnimator* meshAnimator = obj->GetComponent<MeshAnimator>();

                        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(viewProjTransform * transform->GetWorldTransformMatrix()));

                        if (meshAnimator && meshAnimator->GetBoneTransforms().size())
                        {
                            glUniform1i(isAnimatedLoc, 1);

                            std::vector<Matrix4> const& boneTransforms = meshAnimator->GetBoneTransforms();

                            glUniformMatrix4fv(boneTransformsLoc, (GLsizei)boneTransforms.size(), GL_FALSE, ValuePtrFloat(*boneTransforms.data()));
                        }
                        else
                        {
                            glUniform1i(isAnimatedLoc, 0);
                        }

                        glBindVertexArray(mesh->GetVAO());

                        // Sub meshes in mesh
                        std::vector<MeshEntry> const& entries = mesh->GetMeshEntries();

                        for (MeshEntry const& entry : entries)
                        {
                            // glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(viewProjTransform * transform->GetWorldTransformMatrix() * entry.m_MeshTransform));
                            glUniformMatrix4fv(nodeTransformLoc, 1, GL_FALSE, ValuePtrFloat(entry.m_NodeTransform));

                            glDrawElementsBaseVertex(GL_TRIANGLES, entry.m_Size, GL_UNSIGNED_INT, (void*)(entry.m_BaseIndex * sizeof(GLuint)), entry.m_BaseVertex);
                        }
                    }
                }
            }

            glDisable(GL_CULL_FACE);
            glDisable(GL_STENCIL_TEST);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glLineWidth(1);
        }
    }
}

void DebugPass::DrawDebug(unsigned layerId, std::vector<Vector3> const& geometry, Color4 color, DebugPrimitiveType type)
{
    std::vector<DebugDrawCommand>& commandList = m_DebugDrawCommands[layerId];
    std::vector<Vector3>& geometryList = m_DebugDrawVertices[layerId];

    commandList.push_back(DebugDrawCommand { static_cast<int>(geometryList.size()), static_cast<int>(geometry.size()), color, type });
    geometryList.insert(geometryList.end(), geometry.begin(), geometry.end());
}

void DebugPass::DrawCube(unsigned layerId, Color4 color)
{
    DrawDebug(layerId, gSolidBoxMesh, color, DebugPrimitiveType::Triangles);
}

void DebugPass::DrawDebugBox(unsigned layerId, Color4 color)
{
    DrawDebug(layerId, gWireBoxMesh, color, DebugPrimitiveType::Lines);
}

void DebugPass::DrawSelectionBox(float left, float right, float top, float bottom)
{
    // NDC coordinates
    float leftNDC = (left / m_ViewportSize.x) - 1;
    float rightNDC = (right / m_ViewportSize.x) - 1;
    float topNDC = (top / m_ViewportSize.y) - 1;
    float bottomNDC = (bottom / m_ViewportSize.y) - 1;

    m_SelectionBoxGeometry[0] = Vector2(leftNDC, bottomNDC);
    m_SelectionBoxGeometry[1] = Vector2(rightNDC, bottomNDC);
    m_SelectionBoxGeometry[2] = Vector2(rightNDC, topNDC);
    m_SelectionBoxGeometry[3] = Vector2(leftNDC, topNDC);

    m_SelectionBoxEnabled = true;
}

void DebugPass::SetSelectedObjects(std::vector<unsigned> const& selectedObjects)
{
    m_SelectedObjects = selectedObjects;
}

void DebugPass::SetGizmoMode(GizmoMode mode)
{
    m_GizmoMode = mode;
}

void DebugPass::SetHoveredGizmo(GizmoType gizmo)
{
    m_HoveredGizmo = gizmo;
}

#endif // !USE_VULKAN