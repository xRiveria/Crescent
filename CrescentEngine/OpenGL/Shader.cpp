#include "CrescentPCH.h"
#include "Shader.h"
#include "OpenGLRenderer.h"
#include "GL/glew.h"

Shader::Shader(const std::string& filePath) : m_FilePath(filePath), m_RendererID(0)
{
    ShaderProgramSource source = ParseShader(filePath);
    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{
    glDeleteProgram(m_RendererID);
}

void Shader::Bind() const
{
    glUseProgram(m_RendererID);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}

void Shader::SetUniform1i(const std::string& name, int value)
{
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniform1f(const std::string& name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

int Shader::GetUniformLocation(const std::string& name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
    {
        return m_UniformLocationCache[name];
    }
    int location = glGetUniformLocation(m_RendererID, name.c_str());
    if (location == -1) //Stripped/Can't Obtain Uniform
    {
        std::cout << "Warning: Uniform " << name << " doesn't exist! \n";
    }
    m_UniformLocationCache[name] = location;
   
    return location;
}

ShaderProgramSource Shader::ParseShader(const std::string& filePath)
{
    std::ifstream stream(filePath);

    enum class ShaderType
    {
        None = -1, Vertex = 0, Fragment = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::None;

    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos) //If we find the word #shader...
        {
            if (line.find("vertex") != std::string::npos)
            {
                type = ShaderType::Vertex;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                type = ShaderType::Fragment;
            }
        }
        else
        {
            //The enum index here is used as the array index. Hence the above manual index assignments. Smart!
            ss[(int)type] << line << "\n";
        }
    }
    return { ss[0].str(), ss[1].str() };
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result); //i specifies we are specifying an integer, and v means it wants an array, aka a pointer.
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char)); //Alloca allows us to allocate on the stack dynamically. 
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader!" << "\n";
        std::cout << message << "\n";
        glDeleteShader(id);
        return 0;
    }

    return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    //Once linked, we can delete the intermediates.
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}