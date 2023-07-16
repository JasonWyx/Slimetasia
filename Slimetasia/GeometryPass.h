#pragma once
#ifndef USE_VULKAN

#include <GL/glew.h>

#include <array>
#include <vector>

#include "CorePrerequisites.h"
#include "MathDefs.h"
#include "RenderLayer.h"
#include "RenderPass.h"

class Camera;
class MeshRenderer;

enum class GBuffer
{
    Diffuse = 0,
    Specular,
    Emissive,
    WorldPosition,
    WorldNormal,
#ifdef EDITOR
    TexCoords,
    PickingID,
#endif
    Count
};

using GBuffers = std::array<GLuint, (int)GBuffer::Count>;

class GeometryPass : public RenderPass
{
public:

    GeometryPass(const iVector2& viewportSize);
    ~GeometryPass();

    void Render(Camera* camera, const RenderLayer& renderLayer) override;
    void BuildRenderTargets() override;

    const GBuffers& GetGBuffers() const;
    GLuint GetFramebuffer() const;
    GLuint GetDepthBuffer() const;
    GLint GetPickedID(iVector2 mousePosition) const;

    void SetClipPlane(const Vector4& clipPlane);
    void StartMeshCulling(const Camera& camera, const RenderLayer& renderLayer);

private:

    HShader m_GeomShader;
    HShader m_TextShader;
    HShader m_VideoShader;

    GLuint m_Framebuffer;
    GBuffers m_GBuffers;

    GLuint m_DSBuffer;
    GLuint m_PixelBuffer;

    Vector4 m_ClipPlane;

    GLuint m_VertexArray;
    GLuint m_VertexBuffer;

    std::future<std::vector<MeshRenderer*>> culledMeshRenderers;
};

#endif // !USE_VULKAN