#pragma once

class Mesh;
class Sphere;

class MeshManager {
public:
    MeshManager(std::vector<std::shared_ptr<Mesh>> meshes);
    MeshManager();

    void addMesh(const std::shared_ptr<Mesh> &mesh);
    void addSphere(std::shared_ptr<Sphere> sphere);
    void removeMesh(unsigned int index);
    void buildSSBOs();

private:
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::shared_ptr<Sphere>> spheres;

    unsigned int vertices_ssbo = 0;
    unsigned int indices_ssbo = 0;
    unsigned int material_ssbo = 0;

    unsigned int spheres_ssbo = 0;
};
