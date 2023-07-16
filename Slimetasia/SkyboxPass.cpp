#ifndef USE_VULKAN

#include "SkyboxPass.h"

#include "ResourceManager.h"

SkyboxPass::SkyboxPass()
    : m_SkyboxVAO(0)
    , m_SkyboxVBO(0)
    , m_SkyboxShader(ResourceManager::Instance().CreateResource<Shader>("SkyboxShader"))
{
    glCreateVertexArrays(1, &m_SkyboxVAO);
    glCreateBuffers(1, &m_SkyboxVBO);

    glVertexArrayVertexBuffer(m_SkyboxVAO, 0, m_SkyboxVBO, 0, sizeof(float) * 3);

    glEnableVertexArrayAttrib(m_SkyboxVAO, 0);
    glVertexArrayAttribBinding(m_SkyboxVAO, 0, 0);
    glVertexArrayAttribFormat(m_SkyboxVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

    const GLfloat bufferData[] = { // positions
                                   -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                                   -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                                   1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                                   -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                                   -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                                   -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f
    };

    glNamedBufferStorage(m_SkyboxVBO, sizeof(bufferData), bufferData, 0);

    m_SkyboxShader->SetVertShaderFilePath("Skybox.vert");
    m_SkyboxShader->SetFragShaderFilePath("Skybox.frag");
    m_SkyboxShader->Compile();
}

SkyboxPass::~SkyboxPass()
{
    glDeleteBuffers(1, &m_SkyboxVBO);
    glDeleteVertexArrays(1, &m_SkyboxVAO);
}

void SkyboxPass::Render(Camera& camera)
{
    HTexture skybox = camera.GetSkyboxTexture();

    if (skybox.Validate())
    {
        glBindVertexArray(m_SkyboxVAO);

        if (m_SkyboxShader->Enable())
        {
            static const GLuint skyboxTextureLoc = glGetUniformLocation(m_SkyboxShader->GetProgramHandle(), "skyboxTex");
            static const GLuint skyboxColorLoc = glGetUniformLocation(m_SkyboxShader->GetProgramHandle(), "skyboxColor");
            static const GLuint viewProjectionLoc = glGetUniformLocation(m_SkyboxShader->GetProgramHandle(), "viewProjection");

            glBindTextureUnit(0, skybox->GetHandle());
            glUniform1i(skyboxTextureLoc, 0);

            Matrix4 viewProjection = camera.GetProjTransform() * Matrix4::LookAt(Vector3(), camera.GetLookAtDirection(), Vector3(0.0f, 1.0f, 0.0f));

            glUniformMatrix4fv(viewProjectionLoc, 1, GL_FALSE, ValuePtrFloat(viewProjection));
            glUniform4fv(skyboxColorLoc, 1, ValuePtrFloat(camera.GetSkyboxColor()));

            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LEQUAL);

            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);

            glDrawArrays(GL_TRIANGLES, 0, 36);

            glDisable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            glDisable(GL_BLEND);
        }
    }
}

#endif // !USE_VULKAN