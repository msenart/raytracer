#pragma once

#include "OpenGLIncludes.h"

class ShaderProgram {
public:
    /// @brief Main function of the class. Generates a shader program using paths to written shaders. Takes care of includes.
    /// If the compute shader path is not empty, the program will be a compute shader. Else, it is a normal rasterization shader.
    /// @param vertexShader         vertex shader path from the executable.
    /// @param fragmentShader       fragment shader path from the executable.
    /// @param geometryShader       geometry shader path from the executable.
    /// @param tessControlShader    tesselation control shader path from the executable.
    /// @param tessEvalShader       tesselation evaluation shader path from the executable.
    /// @param computeShader        compute shader path from the executable.
    /// @return
    static std::shared_ptr<ShaderProgram> genShaderProgram(
    const std::string& vertexShader,
    const std::string& fragmentShader,
    const std::string& geometryShader   = "",
    const std::string& tessControlShader = "",
    const std::string& tessEvalShader    = "",
    const std::string& computeShader     = ""
);

    GLuint id() const { return _id; }

    /// @brief locks the shader program for variable bindings.
    void use() const { glUseProgram(_id); }

    /// @brief unlocks the shader program for variable bindings.
    static void stop() { glUseProgram(0); }

    /// @brief recompiles the shader program following its paths.
    void reload();

    // All the setters for variable binding.

    void set(const std::string &name, int value) const {
        GLint loc = getLocationChecked(name);
        if (loc != -1) glUniform1i(loc, value);
    }

    void set(const std::string &name, float value) const {
        GLint loc = getLocationChecked(name);
        if (loc != -1) glUniform1f(loc, value);
    }

    void set(const std::string &name, GLuint value) const {
        GLint loc = getLocationChecked(name);
        if (loc != -1) glUniform1i(loc, static_cast<GLint>(value));
    }

    void set(const std::string &name, const glm::vec2 &value) const {
        GLint loc = getLocationChecked(name);
        if (loc != -1) glUniform2fv(loc, 1, glm::value_ptr(value));
    }

    void set(const std::string &name, const glm::vec3 &value) const {
        GLint loc = getLocationChecked(name);
        if (loc != -1) glUniform3fv(loc, 1, glm::value_ptr(value));
    }

    void set(const std::string &name, const glm::vec4 &value) const {
        GLint loc = getLocationChecked(name);
        if (loc != -1) glUniform4fv(loc, 1, glm::value_ptr(value));
    }

    void set(const std::string &name, const glm::mat3 &value) const {
        GLint loc = getLocationChecked(name);
        if (loc != -1) glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(value));
    }

    void set(const std::string &name, const glm::mat4 &value) const {
        GLint loc = getLocationChecked(name);
        if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
    }
    bool enableSendNotBoundVariableMessage = true;

    static inline bool allEnableSendNotBoundVariableMessage = true;

    static void allEnableVariableNotBoundMessage(bool b) {
        allEnableSendNotBoundVariableMessage = b;
    }

    void enableVariableNotBoundMessage(bool b) {
        enableSendNotBoundVariableMessage = b;
    }
    virtual ~ShaderProgram();
    ShaderProgram& operator=(ShaderProgram&& other) noexcept {
        if (this != &other) {
            glDeleteProgram(_id);
            _id = other._id;
        }
        return *this;
    }

    ShaderProgram();

    ShaderProgram(ShaderProgram&& other) noexcept {
        _id = other._id;
    }

private:

    // Fonction interne pour récupérer la location et vérifier
    GLint getLocationChecked(const std::string &name) const {
        GLint loc = glGetUniformLocation(_id, name.c_str());
        if (loc == -1 && enableSendNotBoundVariableMessage && allEnableSendNotBoundVariableMessage) {
            std::cout << "Uniform '" << name << "' introuvable !" << std::endl;
        }
        return loc;
    }

    void loadShader(GLenum type, const std::string &shaderFilename);
    void link() const {
        glLinkProgram(_id);

        GLint success;
        glGetProgramiv(_id, GL_LINK_STATUS, &success);

        if (!success) {
            GLint len;
            glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &len);
            if (len > 0) {
                std::vector<GLchar> log(len);
                glGetProgramInfoLog(_id, len, nullptr, log.data());
                std::cerr << "--- PROGRAM LINK ERROR ---" << std::endl;
                std::cerr << log.data() << std::endl;
                std::cerr << "--------------------------" << std::endl;
            }
        }
    }

    std::string file2String(const std::string &filename);

    std::string loadShaderWithIncludes(const std::string &filename, std::unordered_set<std::string> &alreadyIncluded);

    GLuint _id = 0;
    std::string vertexShaderPath;
    std::string fragmentShaderPath;
    std::string geometryShaderPath;
    std::string tessControlShaderPath;
    std::string tessEvalShaderPath;
    std::string computeShaderPath;
};
