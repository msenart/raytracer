#pragma once

#include "OpenGLIncludes.h"

#include "Mesh.h"

class Shader;
class Camera;

/// @brief Which object of the scene is currently selected (by mouse click).
struct Selection {
    enum class Kind { None, Mesh, Sphere };
    Kind kind = Kind::None;
    unsigned int index = 0;
};

class ObjectManager {
public:
    ObjectManager(std::vector<std::shared_ptr<Mesh>> meshes);
    ObjectManager();

    void addMesh(const std::shared_ptr<Mesh> &mesh);
    void addSphere(std::shared_ptr<Sphere> sphere);
    void removeMesh(unsigned int index);
    void removeSphere(unsigned int index);
    [[nodiscard]] unsigned int meshesNumber() const { return meshes.size(); }
    [[nodiscard]] unsigned int spheresNumber() const { return spheres.size(); }
    void buildSSBOs();
    void buildBVH();
    /// @brief Rebuilds the BVH and every SSBO from the current state of the scene (transforms, materials).
    void rebuildAccelerationStructure();
    /// @brief Re-uploads the materials only. Valid as long as no object was added/removed since the last SSBO build.
    void updateMaterials();
    unsigned int primitivesNumber() const;

    /// @brief Draws the scene with the rasterization pipeline using the given shader.
    void drawRaster(const Shader& shader, const Camera& camera);

    /// @brief Selects the closest object hit by the ray (or deselects if nothing is hit).
    /// @param origin world space ray origin.
    /// @param direction world space ray direction, normalized.
    /// @return true if an object was hit.
    bool pick(const glm::vec3& origin, const glm::vec3& direction);

    [[nodiscard]] Mesh* selectedMesh() const;
    [[nodiscard]] Sphere* selectedSphere() const;

    Selection selection;
private:
    struct RasterBuffers {
        unsigned int vao = 0;
        unsigned int vbo = 0;
        unsigned int ebo = 0;
        int count = 0;
    };

    static RasterBuffers createRasterBuffers(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    static void destroyRasterBuffers(RasterBuffers& buffers);
    void ensureRasterBuffers();
    [[nodiscard]] std::vector<Material> collectMaterials() const;

    std::vector<RasterBuffers> mesh_raster;
    RasterBuffers sphere_raster;

    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::shared_ptr<Sphere>> spheres;

    unsigned int vertices_ssbo = 0;     // binding 0
    unsigned int indices_ssbo = 0;      // binding 1
    unsigned int spheres_ssbo = 0;      // binding 2
    unsigned int material_ssbo = 0;     // binding 3
    unsigned int bvh_ssbo = 0;          // binding 4
    unsigned int transform_ssbo = 0;
};
