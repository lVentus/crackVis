#pragma once
#include <vector>
#include <string>
#include <memory>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glowl/glowl.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
    aiString path;
};


namespace OGL4Core2::Plugins::PCVC::CrackVis {
    class CrackVis;

    class Mesh {
    public:
        /*  网格数据  */
        CrackVis& basePlugin;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;
        std::shared_ptr<glowl::GLSLProgram> shaderProgram;
        uint32_t globalID;
        Mesh(CrackVis& basePlugin, std::vector<Vertex> vertices, std::vector<unsigned int> indices,
            std::vector<Texture> textures, uint16_t modelID, uint16_t meshNum);

        void Draw(const glm::mat4& projMx, const glm::mat4& viewMx);

    private:
        unsigned int VAO, VBO, EBO;
        void setupMesh();
    };

    unsigned int TextureFromFile(std::string path, const std::string& directory, bool gamma = false);

    class Model {
    public:
        Model(CrackVis& basePlugin, std::string path) :
            basePlugin(basePlugin),
            shaderProgram(nullptr)
        {
            static uint16_t nextID = 0;
            modelID = nextID++;
            meshNum = 0;
            loadModel(path);
        }
        void Draw(const glm::mat4& projMx, const glm::mat4& viewMx);
        CrackVis& basePlugin;
        std::vector<Texture> textures_loaded;
        uint16_t modelID;
        uint16_t meshNum;
        const std::vector<Mesh>& getMeshes() const {
            return meshes;
        }
        void initShader(std::string vsPath, std::string fspath);

        std::shared_ptr<glowl::GLSLProgram> shaderProgram;

    private:
        std::vector<Mesh> meshes;
        std::string directory;
        void loadModel(std::string path);
        void processNode(aiNode* node, const aiScene* scene);
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    };

} // namespace OGL4Core2::Plugins::PCVC::CrackVis
