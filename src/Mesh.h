#pragma once

#include "OpenGLIncludes.h"
#include "Shader.h"
#include "MeshHelpers.h"
#include "Transform.h"

struct Mesh {
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);

    void uploadUniforms(const Shader& shader) const;

    Transform transform;
    glm::vec4 color;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};