#pragma once

class Mesh;
class Sphere;

class ObjectManager {
public:
    ObjectManager(std::vector<std::shared_ptr<Mesh>> meshes);
    ObjectManager();

    void addMesh(const std::shared_ptr<Mesh> &mesh);
    void addSphere(std::shared_ptr<Sphere> sphere);
    void removeMesh(unsigned int index);
    void buildSSBOs();
    void buildBVH();

private:
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::shared_ptr<Sphere>> spheres;

    unsigned int vertices_ssbo = 0;     // binding 0
    unsigned int indices_ssbo = 0;      // binding 1
    unsigned int spheres_ssbo = 0;      // binding 2
    unsigned int material_ssbo = 0;     // binding 3
    unsigned int bvh_ssbo = 0;          // binding 4
};
