#include "Shader.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

Shader::Shader(const std::string& resourceName, const filesystem::path& filePath)
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
    if (m_ShaderProgram != 0) glDeleteProgram(m_ShaderProgram);
}

void Shader::Compile()
{
    if (m_ShaderProgram != 0) glDeleteProgram(m_ShaderProgram);

    std::ifstream vertShaderFile("Shaders/" + m_VertShaderFilePath, std::ios::in);
    std::ifstream fragShaderFile("Shaders/" + m_FragShaderFilePath, std::ios::in);
    std::ifstream geomShaderFile("Shaders/" + m_GeomShaderFilePath, std::ios::in);

    if (!vertShaderFile.is_open()) throw std::runtime_error("ERROR: " + std::string(m_VertShaderFilePath) + " failed to open.");
    if (!fragShaderFile.is_open()) throw std::runtime_error("ERROR: " + std::string(m_FragShaderFilePath) + " failed to open.");
    if (!m_GeomShaderFilePath.empty() && !geomShaderFile.is_open()) throw std::runtime_error("ERROR: " + std::string(m_GeomShaderFilePath) + " failed to open.");

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint geomShader = glCreateShader(GL_GEOMETRY_SHADER);

    std::string vertSource = "";
    std::string fragSource = "";
    std::string geomSource = "";
    std::string line = "";

    while (std::getline(vertShaderFile, line))
    {
        vertSource += '\n' + line;
    }
    vertShaderFile.close();

    char const* src = vertSource.c_str();

    GLint result = GL_FALSE;
    int infoLength = 0;

    glShaderSource(vertShader, 1, &src, nullptr);
    glCompileShader(vertShader);

    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &infoLength);
    if (infoLength > 0)
    {
        std::vector<char> error(static_cast<std::size_t>(infoLength) + 1);
        glGetShaderInfoLog(vertShader, infoLength, nullptr, error.data());
        std::cout << error.data() << std::endl;
    }

    line = "";
    while (std::getline(fragShaderFile, line))
    {
        fragSource += '\n' + line;
    }
    fragShaderFile.close();

    src = fragSource.c_str();

    result = GL_FALSE;
    infoLength = 0;

    glShaderSource(fragShader, 1, &src, nullptr);
    glCompileShader(fragShader);

    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &infoLength);
    if (infoLength > 0)
    {
        std::vector<char> error(static_cast<std::size_t>(infoLength) + 1);
        glGetShaderInfoLog(fragShader, infoLength, nullptr, error.data());
        std::cout << error.data() << std::endl;
        return;
    }

    if (!m_GeomShaderFilePath.empty() && geomShaderFile.is_open())
    {
        line = "";
        while (std::getline(geomShaderFile, line))
        {
            geomSource += '\n' + line;
        }
        geomShaderFile.close();

        src = geomSource.c_str();

        result = GL_FALSE;
        infoLength = 0;

        glShaderSource(geomShader, 1, &src, nullptr);
        glCompileShader(geomShader);

        glGetShaderiv(geomShader, GL_COMPILE_STATUS, &result);
        glGetShaderiv(geomShader, GL_INFO_LOG_LENGTH, &infoLength);
        if (infoLength > 0)
        {
            std::vector<char> error(static_cast<std::size_t>(infoLength) + 1);
            glGetShaderInfoLog(geomShader, infoLength, nullptr, error.data());
            std::cout << error.data() << std::endl;
            return;
        }
    }

    GLuint tmpProgram = glCreateProgram();
    glAttachShader(tmpProgram, vertShader);
    glAttachShader(tmpProgram, fragShader);
    if (!m_GeomShaderFilePath.empty()) glAttachShader(tmpProgram, geomShader);
    glLinkProgram(tmpProgram);

    result = GL_FALSE;
    infoLength = 0;

    glGetProgramiv(tmpProgram, GL_LINK_STATUS, &result);
    glGetProgramiv(tmpProgram, GL_INFO_LOG_LENGTH, &infoLength);
    if (infoLength > 0)
    {
        std::vector<char> error(static_cast<std::size_t>(infoLength) + 1);
        glGetShaderInfoLog(tmpProgram, infoLength, nullptr, error.data());
        std::cout << error.data() << std::endl;
        return;
    }

    glDetachShader(tmpProgram, vertShader);
    glDetachShader(tmpProgram, fragShader);
    glDetachShader(tmpProgram, geomShader);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    glDeleteShader(geomShader);

    m_ShaderProgram = tmpProgram;
}

bool Shader::Enable()
{
    if (m_ShaderProgram != 0) glUseProgram(m_ShaderProgram);
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
