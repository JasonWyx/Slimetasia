#include "Shader.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

Shader::Shader(const std::string& resourceName, const std::filesystem::path& filePath)
    : ResourceBase(resourceName, filePath)
    , m_ShaderProgram(0)
    , m_VertShaderFilePath()
    , m_FragShaderFilePath()
    , m_GeomShaderFilePath()
    , m_TessShaderFilePath()
{
}

Shader::~Shader()
{
    if (m_ShaderProgram != 0)
    {
        glDeleteProgram(m_ShaderProgram);
    }
}

void Shader::Compile()
{
    if (m_ShaderProgram != 0)
    {
        glDeleteProgram(m_ShaderProgram);
    }

    GLuint vertShader = CompileShader(std::string { "Shaders/" } + m_VertShaderFilePath, GL_VERTEX_SHADER);
    GLuint fragShader = CompileShader(std::string { "Shaders/" } + m_FragShaderFilePath, GL_FRAGMENT_SHADER);
    GLuint geomShader = CompileShader(std::string { "Shaders/" } + m_GeomShaderFilePath, GL_GEOMETRY_SHADER);

    m_ShaderProgram = LinkProgram({ vertShader, fragShader, geomShader });
}

bool Shader::Enable()
{
    if (m_ShaderProgram != 0)
    {
        glUseProgram(m_ShaderProgram);
    }
    return m_ShaderProgram != 0;
}

void Shader::Disable()
{
    glUseProgram(0);
}

GLuint Shader::GetProgramHandle() const
{
    return m_ShaderProgram;
}

std::string const& Shader::GetVertShaderFilePath() const
{
    return m_VertShaderFilePath;
}

void Shader::SetVertShaderFilePath(std::string const& vertShaderFilePath)
{
    m_VertShaderFilePath = vertShaderFilePath;
}

std::string const& Shader::GetFragShaderFilePath() const
{
    return m_FragShaderFilePath;
}

void Shader::SetFragShaderFilePath(std::string const& fragShaderFilePath)
{
    m_FragShaderFilePath = fragShaderFilePath;
}

std::string const& Shader::GetGeomShaderFilePath() const
{
    return m_GeomShaderFilePath;
}

void Shader::SetGeomShaderFilePath(std::string const& geomShaderFilePath)
{
    m_GeomShaderFilePath = geomShaderFilePath;
}

std::string const& Shader::GetTessShaderFilePath() const
{
    return m_TessShaderFilePath;
}

void Shader::SetTessShaderFilePath(std::string const& tessShaderFilePath)
{
    m_TessShaderFilePath = tessShaderFilePath;
}

void Shader::Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* root)
{
    ResourceBase::Serialize(doc, root);
}

void Shader::Unserialize(tinyxml2::XMLElement* root)
{
    ResourceBase::Unserialize(root);
}

/*static*/ GLuint Shader::CompileShader(std::string filePath, GLenum shaderType)
{
    std::ifstream shaderFileStream { filePath, std::ios::in };

    if (!shaderFileStream.is_open())
    {
        std::cout << "ERROR: " + std::string(filePath) + " failed to open." << std::endl;
        return GL_NONE;
    }

    GLuint shader = glCreateShader(shaderType);

    std::string source = "";
    std::string line = "";

    while (std::getline(shaderFileStream, line))
    {
        source += line + '\n';
    }

    shaderFileStream.close();

    char const* sourceCStr = source.c_str();

    GLint result = GL_FALSE;
    int infoLength = 0;

    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);

        std::vector<char> error(static_cast<std::size_t>(infoLength) + 1);
        glGetShaderInfoLog(shader, infoLength, nullptr, error.data());

        std::cout << error.data() << std::endl;

        glDeleteShader(shader);
        return GL_NONE;
    }

    return shader;
}

GLuint Shader::LinkProgram(const std::vector<GLuint>& programs)
{
    GLuint shaderProgram = glCreateProgram();

    for (const GLuint program : programs)
    {
        glAttachShader(shaderProgram, program);
    }

    glLinkProgram(shaderProgram);

    GLint result = GL_FALSE;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &result);

    if (!result)
    {
        GLint infoLength = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLength);

        std::vector<char> error(static_cast<std::size_t>(infoLength) + 1);
        glGetShaderInfoLog(shaderProgram, infoLength, nullptr, error.data());
        std::cout << error.data() << std::endl;

        return GL_NONE;
    }

    for (const GLuint program : programs)
    {
        glDetachShader(shaderProgram, program);
        glDeleteShader(program);
    }

    return shaderProgram;
}
