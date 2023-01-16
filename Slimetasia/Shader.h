#pragma once
#include <GL\glew.h>

#include <string>

#include "ResourceBase.h"

class Shader : public ResourceBase
{
public:

    Shader(const std::string& resourceName = "Shader", const std::filesystem::path& filePath = "");
    ~Shader();

    // Inherited via ResourceBase
    void Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* root) override;
    void Unserialize(tinyxml2::XMLElement* root) override;

    void Compile();
    bool Enable();
    void Disable();

    GLuint GetProgramHandle() const { return m_ShaderProgram; }
    std::string const& GetVertShaderFilePath() const { return m_VertShaderFilePath; }
    void SetVertShaderFilePath(std::string const& vertShaderFilePath) { m_VertShaderFilePath = vertShaderFilePath; }
    std::string const& GetFragShaderFilePath() const { return m_FragShaderFilePath; }
    void SetFragShaderFilePath(std::string const& fragShaderFilePath) { m_FragShaderFilePath = fragShaderFilePath; }
    std::string const& GetGeomShaderFilePath() const { return m_GeomShaderFilePath; }
    void SetGeomShaderFilePath(std::string const& geomShaderFilePath) { m_GeomShaderFilePath = geomShaderFilePath; }
    std::string const& GetTessShaderFilePath() const { return m_TessShaderFilePath; }
    void SetTessShaderFilePath(std::string const& tessShaderFilePath) { m_TessShaderFilePath = tessShaderFilePath; }

private:

    static GLuint CompileShader(std::string filePath, GLenum shaderType);
    GLuint LinkProgram(const std::vector<GLuint>& programs);

    GLuint m_ShaderProgram;
    std::string m_VertShaderFilePath;
    std::string m_FragShaderFilePath;
    std::string m_GeomShaderFilePath;
    std::string m_TessShaderFilePath;
};