#include "GBufferRenderPass.h"

#include "Renderer.h"
#include "ResourceManager.h"
#include "Shader.h"

GBufferRenderPass::GBufferRenderPass()
    : m_Framebuffer(0)
    , m_GBuffers()  //,
                    // m_GBufferShader(ResourceManager::Instance().CreateResource<Shader>())
{
    // m_GBufferShader->SetVertShaderFilePath("GBuffer.vert");
    // m_GBufferShader->SetFragShaderFilePath("GBuffer.frag");
    // m_GBufferShader->Compile();

    iVector2 textureSize = Renderer::Instance().GetViewportSize();

    glGenFramebuffers(1, &m_Framebuffer);
    glGenRenderbuffers(1, &m_GBuffers[GBufferID::Depth]);
    glGenTextures(GBufferID::Depth, m_GBuffers);

    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

    glBindTexture(GL_TEXTURE_2D, m_GBuffers[GBufferID::Diffuse]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureSize.x, textureSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GBuffers[GBufferID::Diffuse], 0);

    glBindTexture(GL_TEXTURE_2D, m_GBuffers[GBufferID::Specular]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize.x, textureSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GBuffers[GBufferID::Specular], 0);

    glBindTexture(GL_TEXTURE_2D, m_GBuffers[GBufferID::Normal]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, textureSize.x, textureSize.y, 0, GL_RGB32F, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GBuffers[GBufferID::Normal], 0);

    glBindTexture(GL_TEXTURE_2D, m_GBuffers[GBufferID::Position]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, textureSize.x, textureSize.y, 0, GL_RGB32F, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_GBuffers[GBufferID::Position], 0);

    glBindRenderbuffer(GL_RENDERBUFFER, m_GBuffers[GBufferID::Depth]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, textureSize.x, textureSize.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_GBuffers[GBufferID::Depth]);
}

GBufferRenderPass::~GBufferRenderPass()
{
    glDeleteTextures(GBufferID::Depth, m_GBuffers);
    glDeleteRenderbuffers(1, &m_GBuffers[GBufferID::Depth]);
    glDeleteFramebuffers(1, &m_Framebuffer);
}

void GBufferRenderPass::Render(Camera& camera, std::vector<MeshRenderer*> const& meshList) {}

void GBufferRenderPass::SetViewportSize(iVector2 size)
{
    glBindTexture(GL_TEXTURE_2D, m_GBuffers[GBufferID::Diffuse]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    glBindTexture(GL_TEXTURE_2D, m_GBuffers[GBufferID::Specular]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindTexture(GL_TEXTURE_2D, m_GBuffers[GBufferID::Normal]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size.x, size.y, 0, GL_RGB32F, GL_UNSIGNED_BYTE, 0);

    glBindTexture(GL_TEXTURE_2D, m_GBuffers[GBufferID::Position]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size.x, size.y, 0, GL_RGB32F, GL_UNSIGNED_BYTE, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, m_GBuffers[GBufferID::Depth]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);
}
