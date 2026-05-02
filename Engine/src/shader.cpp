#include <Engine/shader.h>

static void checkCompile(unsigned int shader, const char* type)
{
    int success = 0;
    char infoLog[1024];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << "ERROR::SHADER_COMPILATION_ERROR (" << type << ")\n"
                  << infoLog << "\n";
    }
}

static void checkLink(unsigned int program)
{
    int success = 0;
    char infoLog[1024];

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM_LINKING_ERROR\n"
                  << infoLog << "\n";
    }
}

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    std::string vertexCode;
    std::string fragmentCode;

    std::ifstream vFile(vertexPath);
    std::ifstream fFile(fragmentPath);

    if (!vFile.is_open() || !fFile.is_open()) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ\n"
                  << "Vertex: " << vertexPath << "\n"
                  << "Fragment: " << fragmentPath << "\n";
        ID = 0;
        return;
    }

    std::stringstream vStream, fStream;
    vStream << vFile.rdbuf();
    fStream << fFile.rdbuf();

    vertexCode = vStream.str();
    fragmentCode = fStream.str();

    const char* vSrc = vertexCode.c_str();
    const char* fSrc = fragmentCode.c_str();

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vSrc, nullptr);
    glCompileShader(vs);
    checkCompile(vs, "VERTEX");

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fSrc, nullptr);
    glCompileShader(fs);
    checkCompile(fs, "FRAGMENT");

    ID = glCreateProgram();
    glAttachShader(ID, vs);
    glAttachShader(ID, fs);
    glLinkProgram(ID);
    checkLink(ID);

    glDeleteShader(vs);
    glDeleteShader(fs);
}

void Shader::use()
{
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
