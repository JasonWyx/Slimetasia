#include "FinalPass.h"

#include "ResourceManager.h"

FinalPass::FinalPass(iVector2 const& viewportSize)
    : m_FinalPassShader(ResourceManager::Instance().CreateResource<Shader>("FinalPassShader"))
    , m_ViewportSize(viewportSize)
    , m_Framebuffer(0)
    , m_RenderTarget(0)
{
    m_FinalPassShader->SetVertShaderFilePath("BasicTransform.vert");
    m_FinalPassShader->SetFragShaderFilePath("FinalPass.frag");
    m_FinalPassShader->Compile();

    glCreateFramebuffers(1, &m_Framebuffer);
    glNamedFramebufferDrawBuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0);

    BuildRenderTargets();

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

FinalPass::~FinalPass()
{
    glDeleteFramebuffers(1, &m_Framebuffer);
    glDeleteTextures(1, &m_RenderTarget);

    glDeleteBuffers(1, &m_VertexBuffer);
    glDeleteVertexArrays(1, &m_VertexArray);
}

void FinalPass::Render(const Camera& camera, GLuint renderTexture)
{
    if (m_FinalPassShader->Enable())
    {
        static const GLuint renderTextureLoc = glGetUniformLocation(m_FinalPassShader->GetProgramHandle(), "gRenderTexture");
        static const GLuint screenSizeLoc = glGetUniformLocation(m_FinalPassShader->GetProgramHandle(), "gScreenSize");
        static const GLuint mvpLoc = glGetUniformLocation(m_FinalPassShader->GetProgramHandle(), "gMVP");
        static const GLuint gammaLoc = glGetUniformLocation(m_FinalPassShader->GetProgramHandle(), "gGamma");
        static const GLuint exposureLoc = glGetUniformLocation(m_FinalPassShader->GetProgramHandle(), "gExposure");

        glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUniform1i(renderTextureLoc, 0);
        glUniform2iv(screenSizeLoc, 1, ValuePtrInt(m_ViewportSize));
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(Matrix4()));
        glUniform1f(gammaLoc, camera.GetGamma());
        glUniform1f(exposureLoc, camera.GetExposure());

        glBindTextureUnit(0, renderTexture);

        glBindVertexArray(m_VertexArray);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
}

void FinalPass::SetViewportSize(iVector2 const& viewportSize)
{
    if (viewportSize != m_ViewportSize)
    {
        m_ViewportSize = viewportSize;
        BuildRenderTargets();
    }
}

GLuint FinalPass::GetRenderTexture() const
{
    return m_RenderTarget;
}

GLuint FinalPass::GetFramebuffer() const
{
    return m_Framebuffer;
}

void FinalPass::BuildRenderTargets()
{
    glDeleteTextures(1, &m_RenderTarget);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_RenderTarget);
    glTextureStorage2D(m_RenderTarget, 1, GL_RGBA16F, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_RenderTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_RenderTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_RenderTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_RenderTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_RenderTarget, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    p_assert(status == GL_FRAMEBUFFER_COMPLETE);
}