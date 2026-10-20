#include "MeshHelpers.h"

#include "OpenGLIncludes.h"
#include "Parameters.h"

void MeshHelpers::generateCube(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices,
    const glm::vec2& tex_density)
{
    vertices.clear();
    indices.clear();

    glm::vec3 p000(-1,-1,-1);
    glm::vec3 p001(-1,-1, 1);
    glm::vec3 p010(-1, 1,-1);
    glm::vec3 p011(-1, 1, 1);
    glm::vec3 p100( 1,-1,-1);
    glm::vec3 p101( 1,-1, 1);
    glm::vec3 p110( 1, 1,-1);
    glm::vec3 p111( 1, 1, 1);

    struct Face {
        glm::vec3 verts[4];
        glm::vec3 normal;
        glm::vec3 tangent;
    };

    Face faces[6] = {
        // top
        { {p010, p110, p111, p011}, glm::vec3(0,1,0), glm::vec3(1,0,0) },
        // bottom
        { {p000, p001, p101, p100}, glm::vec3(0,-1,0), glm::vec3(1,0,0) },
        // front (-Z)
        { {p000, p100, p110, p010}, glm::vec3(0,0,-1), glm::vec3(1,0,0) },
        // back (+Z)
        { {p111, p101, p001, p011}, glm::vec3(0,0,1), glm::vec3(1,0,0) },
        // left (-X)
        { {p010, p011, p001, p000}, glm::vec3(-1,0,0), glm::vec3(0,0,1) },
        // right (+X)
        { {p110, p100, p101, p111}, glm::vec3(1,0,0), glm::vec3(0,0,1) }
    };

    glm::vec2 uv[4] = {
        tex_density * glm::vec2(0,0),
        tex_density * glm::vec2(1,0),
        tex_density * glm::vec2(1,1),
        tex_density * glm::vec2(0,1)
    };

    for (int f = 0; f < 6; ++f) {
        int startIndex = vertices.size();
        for (int v = 0; v < 4; ++v) {
            glm::vec4 tangent4(faces[f].tangent, 1.0f);
            vertices.push_back({faces[f].verts[v], faces[f].normal, tangent4, uv[v] });
        }

        // Deux triangles par face
        indices.push_back(startIndex + 0);
        indices.push_back(startIndex + 2);
        indices.push_back(startIndex + 1);

        indices.push_back(startIndex + 0);
        indices.push_back(startIndex + 3);
        indices.push_back(startIndex + 2);
    }
}

void MeshHelpers::generateSphere(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices,
    const glm::vec2& tex_density,
    int resolution)
{
    const int parallels_count = resolution;
    const int meridian_count = resolution;

    vertices.clear();
    indices.clear();

    for (int i = 0; i <= parallels_count; i++) {
        float parallel = M_PI * i / parallels_count;
        float sinParallel = std::sin(parallel);
        float cosParallel = std::cos(parallel);

        for (int j = 0; j <= meridian_count; j++) {
            float meridian = 2.0f * M_PI * j / meridian_count;
            float sinMeridian = std::sin(meridian);
            float cosMeridian = std::cos(meridian);

            // Position
            glm::vec3 pos(
                cosMeridian * sinParallel,
                sinMeridian * sinParallel,
                cosParallel
            );

            // Normal
            glm::vec3 normal = glm::normalize(pos);

            // Tangente (approximation dérivée par rapport à U)
            glm::vec3 tangent(
                -sinMeridian * sinParallel,
                 cosMeridian * sinParallel,
                  0.0f
            );
            tangent = glm::normalize(tangent);

            // Bitangente et handedness
            glm::vec3 bitangent = glm::cross(normal, tangent);
            float w = (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;
            glm::vec4 tangent4(tangent, w);

            // UV
            glm::vec2 uv(tex_density[0] * (meridian / (2.0f * M_PI)),
                         tex_density[1] * (parallel / M_PI));

            // Vertex
            vertices.push_back({ pos, normal, tangent4, uv });
        }
    }

    // Indices
    for (int i = 0; i < parallels_count; ++i) {
        for (int j = 0; j < meridian_count; ++j) {
            int first  = i * (meridian_count + 1) + j;
            int second = first + meridian_count + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

void MeshHelpers::generatePlane(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices,
    const glm::vec2& tex_density,
    bool doubleSided)
{
    vertices.clear();
    indices.clear();

    glm::vec3 p0(-1, 0, -1);
    glm::vec3 p1( 1, 0, -1);
    glm::vec3 p2( 1, 0,  1);
    glm::vec3 p3(-1, 0,  1);

    struct Face {
        glm::vec3 verts[4];
        glm::vec3 normal;
        glm::vec3 tangent;
    };

    int numFaces = doubleSided ? 2 : 1;
    Face faces[2] = {
        // top face (+Y)
        { {p0, p1, p2, p3}, glm::vec3(0,1,0), glm::vec3(1,0,0) },
        // bottom face (-Y)
        { {p0, p1, p2, p3}, glm::vec3(0,-1,0), glm::vec3(1,0,0) }
    };

    glm::vec2 uv[4] = {
        tex_density * glm::vec2(0,0),
        tex_density * glm::vec2(1,0),
        tex_density * glm::vec2(1,1),
        tex_density * glm::vec2(0,1)
    };

    for (int f = 0; f < numFaces; ++f) {
        int startIndex = vertices.size();
        for (int v = 0; v < 4; ++v) {
            glm::vec4 tangent4(faces[f].tangent, 1.0f);
            vertices.push_back({ faces[f].verts[v], faces[f].normal, tangent4, uv[v] });
        }

        // Two triangles per face
        indices.push_back(startIndex + 2);
        indices.push_back(startIndex + 1);
        indices.push_back(startIndex + 0);

        indices.push_back(startIndex + 0);
        indices.push_back(startIndex + 3);
        indices.push_back(startIndex + 2);
    }
}

void MeshHelpers::generateFromFile(const std::string& path, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) {
    vertices.clear();
    indices.clear();

    std::cout << "Geometry::generateFromFile : loading " << path << std::endl;
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp error : " << importer.GetErrorString() << std::endl;
        return;
    }

    for (int mesh_i = 0; mesh_i < scene->mNumMeshes; ++mesh_i) {
        aiMesh* mesh = scene->mMeshes[mesh_i];
        vertices.reserve(vertices.size() + mesh->mNumVertices);
        indices.reserve(indices.size() + mesh->mNumFaces * 3);

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex{};
            vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

            if (mesh->HasNormals())
                vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

            if (mesh->HasTextureCoords(0))
                vertex.uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

            if (mesh->HasTangentsAndBitangents())
                vertex.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, 1.0f };

            vertices.push_back(vertex);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        std::cout << "Geometry : " << vertices.size() << " vertices, " << indices.size() / 3 << " faces." << std::endl;
    }
}



