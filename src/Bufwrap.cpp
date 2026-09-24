#include "Bufwrap.h"

// ----------------------------------- VBO --------------------------------------

VBO::VBO() {
    glGenBuffers(1, &m_id);
}

VBO::~VBO() {
    glDeleteBuffers(1, &m_id);
}

void VBO::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void VBO::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::setData(GLsizeiptr size, const void *data, GLenum usage) const {
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

void VBO::updateData(GLintptr offset, GLsizeiptr size, const void *data) const {
    bind();
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}

// ----------------------------------- VAO --------------------------------------

VAO::VAO() {
    glGenVertexArrays(1, &m_id);
}

VAO::~VAO() {
    glDeleteVertexArrays(1, &m_id);
}

void VAO::bind() const {
    glBindVertexArray(m_id);
}

void VAO::unbind() {
    glBindVertexArray(0);
}

void VAO::addVBO(const VBO &vbo, const std::vector<VertexAttribute> &layout) const {
    bind();
    vbo.bind();

    for (const auto& attr : layout) {
        glEnableVertexAttribArray(attr.index);
        glVertexAttribPointer(
            attr.index,
            attr.count,
            attr.type,
            attr.normalized,
            attr.stride,
            attr.offset
        );
    }
}

// ----------------------------------- IBO --------------------------------------

IBO::IBO() {
    glGenBuffers(1, &m_id);
    bind();
}

IBO::~IBO() {
    glDeleteBuffers(1, &m_id);
}

void IBO::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

void IBO::unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IBO::setData(GLsizeiptr size, const void *data, GLenum usage) {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
    m_count = size / sizeof(unsigned int);
}

GLuint IBO::count() const { return m_count; }

GLuint IBO::id() const { return m_id; }

// ----------------------------------- RBO --------------------------------------

RBO::RBO() {
    glGenRenderbuffers(1, &m_id);
}

RBO::~RBO() {
    if (m_id != 0)
        glDeleteRenderbuffers(1, &m_id);
}

RBO::RBO(RBO&& other) noexcept {
    m_id = other.m_id;
    other.m_id = 0;
}

RBO& RBO::operator=(RBO&& other) noexcept {
    if (this != &other) {
        glDeleteRenderbuffers(1, &m_id);
        m_id = other.m_id;
        other.m_id = 0;
    }
    return *this;
}

void RBO::bind() const {
    glBindRenderbuffer(GL_RENDERBUFFER, m_id);
}

void RBO::unbind() {
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void RBO::storage(GLenum internalFormat, int width, int height) {
    bind();
    glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
    unbind();
}

// ----------------------------------- FBO --------------------------------------

FBO::FBO() {
    glGenFramebuffers(1, &m_id);
}

FBO::~FBO() {
    if (m_id != 0)
        glDeleteFramebuffers(1, &m_id);
}

FBO::FBO(FBO&& other) noexcept {
    m_id = other.m_id;
    other.m_id = 0;
}

FBO& FBO::operator=(FBO&& other) noexcept {
    if (this != &other) {
        glDeleteFramebuffers(1, &m_id);
        m_id = other.m_id;
        other.m_id = 0;
    }
    return *this;
}

void FBO::bind(GLenum target) const {
    glBindFramebuffer(target, m_id);
}

void FBO::unbind(GLenum target) {
    glBindFramebuffer(target, 0);
}

void FBO::attachTexture2D(GLuint textureID, GLenum attachment, int level) const {
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, textureID, level);
    unbind();
}

void FBO::attachRBO(GLuint rboID, GLenum attachment) const {
    bind();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rboID);
    unbind();
}

bool FBO::isComplete(GLenum target) const {
    bind(target);
    bool ok = (glCheckFramebufferStatus(target) == GL_FRAMEBUFFER_COMPLETE);
    unbind(target);
    return ok;
}