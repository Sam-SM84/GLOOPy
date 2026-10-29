#pragma once
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"
#include "Transform.h"
#include "Light.h"
class Model
{
public:
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;

    std::string directory;
    bool gammaCorrection;
    Transform transform;

    Model(const std::string& path, bool gamma = false);

    void draw(Shader& shader,const Light& ray);
    void free();


private:
    void processNode(aiNode* node, const aiScene* scene);

    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    void loadMaterialTextures(aiMaterial* material, aiTextureType type, const std::string& typeName, std::vector<Texture>& textures);
    
};