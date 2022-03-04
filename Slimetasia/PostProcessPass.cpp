#include "PostProcessPass.h"

#include "Input.h"
#include "ResourceManager.h"

PostProcessPass::PostProcessPass(iVector2 const& viewportSize)
    : m_BasicDrawShader(ResourceManager::Instance().CreateResource<Shader>("PostProcessPassShader"))
    , m_BloomPrePassShader(ResourceManager::Instance().CreateResource<Shader>("BloomPrePassShader"))
    , m_GausBlurShader(ResourceManager::Instance().CreateResource<Shader>("GausianBlurShader"))
    , m_ViewportSize(viewportSize)
    , m_Framebuffer(0)
    , m_BloomPrePassTarget(0)
{
    m_BasicDrawShader->SetVertShaderFilePath("BasicTransform.vert");
    m_BasicDrawShader->SetFragShaderFilePath("BasicDraw.frag");
    m_BasicDrawShader->Compile();

    m_BloomPrePassShader->SetVertShaderFilePath("BasicTransform.vert");
    m_BloomPrePassShader->SetFragShaderFilePath("BloomPrePass.frag");
    m_BloomPrePassShader->Compile();

    m_GausBlurShader->SetVertShaderFilePath("BasicTransform.vert");
    m_GausBlurShader->SetFragShaderFilePath("GaussianBlur.frag");
    m_GausBlurShader->Compile();

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

PostProcessPass::~PostProcessPass()
{
    glDeleteFramebuffers(1, &m_Framebuffer);
    glDeleteTextures(1, &m_BloomPrePassTarget);
    glDeleteTextures(1, &m_BloomPostProcessTarget);

    glDeleteBuffers(1, &m_VertexBuffer);
    glDeleteVertexArrays(1, &m_VertexArray);
}

void PostProcessPass::Render(Camera& camera, GLuint sceneTexture, GLuint mainFramebuffer)
{
    if (camera.IsBloomEnabled())
    {
        // Pre pass to identify bright pixels
        if (m_BloomPrePassShader->Enable())
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
            glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_BloomPrePassTarget, 0);

            static const GLuint mvpLoc = glGetUniformLocation(m_BloomPrePassShader->GetProgramHandle(), "gMVP");
            static const GLuint sceneTextureLoc = glGetUniformLocation(m_BloomPrePassShader->GetProgramHandle(), "gTexture");
            static const GLuint screenSizeLoc = glGetUniformLocation(m_BloomPrePassShader->GetProgramHandle(), "gScreenSize");

            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(Matrix4()));
            glUniform1i(sceneTextureLoc, 0);
            glUniform2iv(screenSizeLoc, 1, ValuePtrInt(m_ViewportSize));

            glBindTextureUnit(0, sceneTexture);

            glBindVertexArray(m_VertexArray);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }

        // Gaussian blur pass
        if (m_GausBlurShader->Enable())
        {
            static const GLuint mvpLoc = glGetUniformLocation(m_GausBlurShader->GetProgramHandle(), "gMVP");
            static const GLuint prepassTexture = glGetUniformLocation(m_GausBlurShader->GetProgramHandle(), "gTexture");
            static const GLuint screenSizeLoc = glGetUniformLocation(m_GausBlurShader->GetProgramHandle(), "gScreenSize");
            static const GLuint horizontalLoc = glGetUniformLocation(m_GausBlurShader->GetProgramHandle(), "gHorizontal");

            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(Matrix4()));
            glUniform1i(prepassTexture, 0);
            glUniform2iv(screenSizeLoc, 1, ValuePtrInt(m_ViewportSize));

            // First pass - horizontal
            glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_BloomPostProcessTarget, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            glUniform1i(horizontalLoc, 1);
            glBindTextureUnit(0, m_BloomPrePassTarget);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            // Second pass - vertical
            glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_BloomPrePassTarget, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            glUniform1i(horizontalLoc, 0);
            glBindTextureUnit(0, m_BloomPostProcessTarget);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }

        if (m_BasicDrawShader->Enable())
        {
            static const GLuint mvpLoc = glGetUniformLocation(m_BasicDrawShader->GetProgramHandle(), "gMVP");
            static const GLuint textureLoc = glGetUniformLocation(m_BasicDrawShader->GetProgramHandle(), "gTexture");

            glBindFramebuffer(GL_FRAMEBUFFER, mainFramebuffer);

            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(Matrix4()));
            glUniform1i(textureLoc, 0);

            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE);

            glBindTextureUnit(0, m_BloomPrePassTarget);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            glDisable(GL_BLEND);
        }
    }
}

void PostProcessPass::SetViewportSize(iVector2 const& viewportSize)
{
    if (viewportSize != m_ViewportSize)
    {
        m_ViewportSize = viewportSize;
        BuildRenderTargets();
    }
}

void PostProcessPass::BuildRenderTargets()
{
    glDeleteTextures(1, &m_BloomPrePassTarget);
    glDeleteTextures(1, &m_BloomPostProcessTarget);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_BloomPrePassTarget);
    glTextureStorage2D(m_BloomPrePassTarget, 1, GL_RGBA16F, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_BloomPrePassTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_BloomPrePassTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_BloomPrePassTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_BloomPrePassTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_BloomPostProcessTarget);
    glTextureStorage2D(m_BloomPostProcessTarget, 1, GL_RGBA16F, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_BloomPostProcessTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_BloomPostProcessTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_BloomPostProcessTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_BloomPostProcessTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    // ASSERT(status == GL_FRAMEBUFFER_COMPLETE);
}
