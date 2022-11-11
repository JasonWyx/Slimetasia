#pragma once
#include <GL\glew.h>

#include <string>

#include "ResourceBase.h"

class Shader : public ResourceBase
{
public:

    Shader(const std::string& resourceName = "Shader", const std::filesystem::path& filePath = "");
    ~Shader();

    void Compile();
    bool Enable();
    void Disable();

    GLuint GetProgramHandle() const;

    std::string const& GetVertShaderFilePath() const;
    void SetVertShaderFilePath(std::string const& vertShaderFilePath);

    std::string const& GetFragShaderFilePath() const;
    void SetFragShaderFilePath(std::string const& fragShaderFilePath);

    std::string const& GetGeomShaderFilePath() const;
    void SetGeomShaderFilePath(std::string const& geomShaderFilePath);

    std::string const& GetTessShaderFilePath() const;
    void SetTessShaderFilePath(std::string const& tessShaderFilePath);

    // Inherited via ResourceBase
    virtual void Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* root) override;
    virtual void Unserialize(tinyxml2::XMLElement* root) override;

private:

    static GLuint CompileShader(std::string filePath, GLenum shaderType);
    GLuint LinkProgram(const std::vector<GLuint>& programs);

    GLuint m_ShaderProgram;
    std::string m_VertShaderFilePath;
    std::string m_FragShaderFilePath;
    std::string m_GeomShaderFilePath;
    std::string m_TessShaderFilePath;
};