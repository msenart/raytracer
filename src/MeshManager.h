#pragma once

class Mesh;

class MeshManager {
public:
    MeshManager(std::vector<std::shared_ptr<Mesh>> meshes);
    MeshManager();

    void addMesh(std::shared_ptr<Mesh> mesh);
    void removeMesh(unsigned int index);
    void buildSSBOs();

private:
    std::vector<std::shared_ptr<Mesh>> meshes;

    unsigned int vertices_ssbo = 0;
    unsigned int indices_ssbo = 0;
    unsigned int material_ssbo = 0;
};
