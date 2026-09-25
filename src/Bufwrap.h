#pragma once

#include <vector>
#include "OpenGLIncludes.h"

class VAO;
class VBO;
class IBO;

struct VertexAttribute {
    GLuint index;
    GLint  count;
    GLenum type;
    GLboolean normalized;
    GLsizei stride;
    const void* offset;
};

class VBO {
public:
    VBO();

    ~VBO();

    void bind() const;

    static void unbind();

    void setData(GLsizeiptr size, const void* data, GLenum usage = GL_STATIC_DRAW) const;

    void updateData(GLintptr offset, GLsizeiptr size, const void* data) const;

private:
    GLuint m_id{};
};

class VAO {
public:
     VAO();

     ~VAO();

     void bind() const;

     static void unbind();

     void addVBO(const VBO& vbo, const std::vector<VertexAttribute>& layout) const;

 private:
    GLuint m_id{};
};

class IBO {
public:
    IBO();

    ~IBO();

    void bind() const;

    static void unbind();

    void setData(GLsizeiptr size, const void* data, GLenum usage = GL_STATIC_DRAW);

    GLuint count() const;

    GLuint id() const;

private:
    GLuint m_id{};
    GLuint m_count{};
};

class RBO {
public:
    RBO();
    ~RBO();

    // Pas de copie
    RBO(const RBO&) = delete;
    RBO& operator=(const RBO&) = delete;

    // Déplacement
    RBO(RBO&& other) noexcept;
    RBO& operator=(RBO&& other) noexcept;

    void bind() const;
    static void unbind();

    void storage(GLenum internalFormat, int width, int height);

    GLuint id() const { return m_id; }

private:
    GLuint m_id = 0;
};

class FBO {
public:
    FBO();
    ~FBO();

    // Pas de copie
    FBO(const FBO&) = delete;
    FBO& operator=(const FBO&) = delete;

    // Déplacement
    FBO(FBO&& other) noexcept;
    FBO& operator=(FBO&& other) noexcept;

    void bind(GLenum target = GL_FRAMEBUFFER) const;
    static void unbind(GLenum target = GL_FRAMEBUFFER);

    void attachTexture2D(GLuint textureID,
                         GLenum attachment = GL_COLOR_ATTACHMENT0,
                         int level = 0) const;

    void attachRBO(GLuint rboID, GLenum attachment = GL_DEPTH_STENCIL_ATTACHMENT) const;

    bool isComplete(GLenum target = GL_FRAMEBUFFER) const;

    GLuint id() const { return m_id; }

private:
    GLuint m_id = 0;
};

// ----------------------------------- UBO --------------------------------------

template<typename T>
class UBO {
public:
    UBO(GLuint bindingPoint);

    ~UBO();

    UBO(const UBO&) = delete;
    UBO& operator=(const UBO&) = delete;

    UBO(UBO&& other) noexcept;

    UBO& operator=(UBO&& other) noexcept;

    void setData(const T& data);

    void setData(const std::vector<T> &vec);

    void bind() const;

    GLuint id() const;

private:
    GLuint m_ubo = 0;
    GLuint m_bindingPoint = 0;
};

template<typename T>
UBO<T>::UBO(GLuint bindingPoint): m_bindingPoint(bindingPoint) {
    glGenBuffers(1, &m_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // bind au binding point
    glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, m_ubo);
}

template<typename T>
UBO<T>::~UBO() {
    if (m_ubo != 0)
        glDeleteBuffers(1, &m_ubo);
}

template<typename T>
UBO<T>::UBO(UBO &&other) noexcept {
    m_ubo = other.m_ubo;
    m_bindingPoint = other.m_bindingPoint;
    other.m_ubo = 0;
}

template<typename T>
UBO<T> & UBO<T>::operator=(UBO &&other) noexcept {
    if (this != &other) {
        glDeleteBuffers(1, &m_ubo);
        m_ubo = other.m_ubo;
        m_bindingPoint = other.m_bindingPoint;
        other.m_ubo = 0;
    }
    return *this;
}

template<typename T>
void UBO<T>::setData(const T &data) {
    glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), &data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

template<typename T>
void UBO<T>::setData(const std::vector<T>& vec)
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
    glBufferSubData(
        GL_UNIFORM_BUFFER,
        0,
        vec.size() * sizeof(T),
        vec.data()
    );
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

template<typename T>
void UBO<T>::bind() const {
    glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, m_ubo);
}

template<typename T>
GLuint UBO<T>::id() const { return m_ubo; }
