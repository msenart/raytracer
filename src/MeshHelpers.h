#pragma once

#include "OpenGLIncludes.h"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 tangent;
    glm::vec2 uv;

    Vertex(glm::vec3 position, glm::vec3 normal, glm::vec4 tangent, glm::vec2 uv) :
    position(position),
    normal(normal),
    tangent(tangent),
    uv(uv) {}

    Vertex() = default;

    bool operator==(const Vertex& other) const {
        return position == other.position && normal == other.normal && tangent == other.tangent && uv == other.uv;
    }
};

namespace MeshHelpers {

    /**
     * @brief               Erases what was in vertices/indices and loads the triangles to make a cube into vertices and indices list. Cube coordinates are in [-1, 1].
     * @param vertices      empty vertices list to be filled.
     * @param indices       empty indices list to be filled.
     * @param tex_density   controls the uv coordinates domain. transforms the coordinates to be in [0, tex_density].
     */
    void generateCube(
        std::vector<Vertex>& vertices,
        std::vector<unsigned int>& indices,
        const glm::vec2& tex_density = glm::vec2(1.0f)
    );

    /**
     * @brief               Erases what was in vertices/indices and loads the triangles to make a sphere into vertices and indices list. Sphere coordinates are in [-1, 1].
     * @param vertices      empty vertices list to be filled.
     * @param indices       empty indices list to be filled.
     * @param tex_density   controls the uv coordinates domain. transforms the coordinates to be in [0, tex_density].
     */
    void generateSphere(
        std::vector<Vertex>& vertices,
        std::vector<unsigned int>& indices,
        const glm::vec2& tex_density = glm::vec2(1.0f),
        int resolution = 16
    );

    /**
     * @brief               Erases what was in vertices/indices and loads the triangles to make a plane into vertices and indices list. Plane coordinates are in [-1, 1].
     * @param vertices      empty vertices list to be filled.
     * @param indices       empty indices list to be filled.
     * @param tex_density   controls the uv coordinates domain. transforms the coordinates to be in [0, tex_density].
     * @param doubleSided   adds a new face to the plane by adding new indices in the indices list.
     */
    void generatePlane(
        std::vector<Vertex>& vertices,
        std::vector<unsigned int>& indices,
        const glm::vec2& tex_density = glm::vec2(1.0f),
        bool doubleSided = false
    );

    /**
     * @brief               Erases what was in vertices/indices and loads the triangles from a path into vertices and indices list.
     * @param path          path of the file to be loaded.
     * @param vertices      empty vertices list to be filled.
     * @param indices       empty indices list to be filled.
     */
    void generateFromFile(const std::string& path, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);
};