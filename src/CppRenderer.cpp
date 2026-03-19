#include "CppRenderer.h"
#include <glad/glad.h>
#include <iostream>

const char* vertex = R"(
    #version 460 core

    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;

    out vec2 texCoord;

    void main()
    {
        texCoord = aTexCoord;
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
)";

const char* fragment = R"(
    #version 460 core

    in vec2 texCoord;
    out vec4 fragColor;

    uniform sampler2D webTexture;

    void main()
    {
        fragColor = texture(webTexture, texCoord);
        //fragColor = vec4(texCoord, 0.0f, 1.0f);
    }
)";

void CheckShaderCompile(GLuint shader, const std::string& type) {
    GLint success;
    GLchar infoLog[1024];
    if (type == "PROGRAM") {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << std::endl;
        }
    }
    else {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << std::endl;
        }
    }
}

void CppRenderer::Init()
{
    {
        glGenBuffers(1, &m_VBO);
    
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    
        float vertices[] = {
            -1.0f, -1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 1.0f,
             1.0f,  1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f,  0.0f, 0.0f
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }
    
    {
        glGenBuffers(1, &m_EBO);
    
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    
        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }
    
    {
        glGenVertexArrays(1, &m_VAO);
    
        glBindVertexArray(m_VAO);
    
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
        glEnableVertexAttribArray(0);
        
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
    }
    
    {
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

        glShaderSource(vertexShader, 1, &vertex, NULL);

        glCompileShader(vertexShader);

        CheckShaderCompile(vertexShader, "VERTEX");

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(fragmentShader, 1, &fragment, NULL);

        glCompileShader(fragmentShader);

        CheckShaderCompile(fragmentShader, "FRAGMENT");

        m_Shader = glCreateProgram();

        glAttachShader(m_Shader, vertexShader);

        glAttachShader(m_Shader, fragmentShader);

        glLinkProgram(m_Shader);

        CheckShaderCompile(m_Shader, "PROGRAM");

        glDeleteShader(vertexShader);

        glDeleteShader(fragmentShader);
    }
}

void CppRenderer::Render(uint32_t texture)
{
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_Shader);
    
    glBindVertexArray(m_VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, texture);

    glUniform1i(glGetUniformLocation(m_Shader, "webTexture"), 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}