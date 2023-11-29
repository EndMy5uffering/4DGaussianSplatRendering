#include "Shader.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "GLEW/glew.h"
#include "Renderer.h"


Shader::Shader() { }

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_RendererID));
}


std::string Shader::LoadShaderSource(const std::string& filePath)
{
#ifdef DEBUG_OUTPUT
    std::cout << "[SHADER]: Loading shader: " << filePath << "\n";
#endif // DEBUG_OUTPUT

    std::ifstream stream(filePath);

    std::string line;

    std::stringstream ss;
    while (std::getline(stream, line))
    {
        ss << line << "\n";
    }

    return ss.str();
}

int Shader::CompileShader(const ShaderSource& shaderSource)
{
    unsigned int id = glCreateShader(shaderSource.type);
    const char* src = shaderSource.source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int len;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
        char* message = (char*)alloca(len * sizeof(char));
        glGetShaderInfoLog(id, len, &len, message);

        std::cout << "Failed to compile\n";
        std::cout << message << std::endl;

        glDeleteShader(id);
        return 0;
    }

    return id;
}

void Shader::Bind() const
{
    if (m_RendererID != 0) 
    {
        GLCall(glUseProgram(m_RendererID));
    }
    else 
    {
        std::cout << "[SHADER ERROR]: RendererID invalid\n";
        std::cout << "[SHADER ERROR]: Make sure that the shader was build before binding it!\n";
        std::cout << "[SHADER ERROR]: To prevent further errors the shader was not bound!\n";
    }
}

void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void Shader::AddShaderSource(const std::string &path, ShaderType type)
{
    std::string src = LoadShaderSource(path);

    if (src.length() == 0) 
    {
        std::cout << "[SHADER ERROR]: Shader with path: " << path << " was empty!\n";
        std::cout << "[SHADER ERROR]: Shader will not be added\n";
    }
    else 
    {
        m_ShaderSources.push_back({ type, path, src });
    }

}

void Shader::BuildShader()
{
    unsigned int program = glCreateProgram();
    std::vector<unsigned int> shaderIds;

    for (ShaderSource ss : m_ShaderSources) 
    {
        unsigned int sid = CompileShader(ss);
        shaderIds.push_back(sid);
        glAttachShader(program, sid);
    }

    glLinkProgram(program);
    glValidateProgram(program);

    for (unsigned int id : shaderIds) glDeleteShader(id);

    m_RendererID = program;
}

inline unsigned int Shader::GetUniformLocation(const std::string& name)
{
    if (uniformCash.find(name) != uniformCash.end())
        return uniformCash[name];

    GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));

    if (location == -1) 
    {
        std::cout << "Warning: uniform " << name << " not found!\n";
    }

    uniformCash[name] = location;
    return location;
}

void Shader::SetUniform1f(const std::string& name, float v0)
{
    GLCall(glUniform1f(GetUniformLocation(name), v0));
}

void Shader::SetUniform2f(const std::string& name, float v0, float v1)
{
    GLCall(glUniform2f(GetUniformLocation(name), v0, v1));
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
    GLCall(glUniform3f(GetUniformLocation(name), v0, v1, v2));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

void Shader::SetUniformMat4f(const std::string& name, glm::mat4 mat)
{
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]));
}

void Shader::SetUniformMat3f(const std::string& name, glm::mat3 mat)
{
    GLCall(glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]));
}

void Shader::SetUniformMat2f(const std::string& name, glm::mat2 mat)
{
    GLCall(glUniformMatrix2fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]));
}

void Shader::SetUniform1i(const std::string& name, int unit)
{
    GLCall(glUniform1i(GetUniformLocation(name), unit));
}

void Shader::DispatchCompute(unsigned int width, unsigned int height, unsigned int depth)
{
    glDispatchCompute(width, height, depth);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void Shader::DispatchCompute(unsigned int width, unsigned int height)
{
    DispatchCompute(width, height, 1);
}

void Shader::DispatchCompute(Texture &tex, unsigned int depth)
{
    DispatchCompute(tex.GetWidth(), tex.GetHeight(), depth);
}

void Shader::DispatchCompute(Texture &tex)
{
    DispatchCompute(tex.GetWidth(), tex.GetHeight(), 1);
}

