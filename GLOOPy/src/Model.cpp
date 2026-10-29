#include "Model.h"

Model::Model(const std::string& path, bool gamma)
{
    gammaCorrection = gamma;
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace
    );

    if (!scene ||
        (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) ||
        !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP:: "
            << importer.GetErrorString() << '\n';
        return;
    }

    const std::size_t separator = path.find_last_of("/\\");

    directory = (separator == std::string::npos)
        ? "."
        : path.substr(0, separator);

    processNode(scene->mRootNode, scene);
}

void Model::draw(Shader& shader,const Light& ray)
{

    shader.setMat4("model", transform.modelMatrix);

    for (Mesh& mesh : meshes)
        mesh.Draw(shader,ray);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        meshes.emplace_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    vertices.reserve(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex;

        vertex.position = glm::vec3(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );

        if (mesh->HasNormals())
        {
            vertex.normal = glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );
        }

        if (mesh->mTextureCoords[0])
        {
            vertex.textureCoordinate = glm::vec2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            );
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace& face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; ++j)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    if (mesh->mMaterialIndex < scene->mNumMaterials)
    {
        aiMaterial* material =
            scene->mMaterials[mesh->mMaterialIndex];

        loadMaterialTextures(
            material,
            aiTextureType_DIFFUSE,
            "texture_diffuse",
            textures
        );

        loadMaterialTextures(
            material,
            aiTextureType_SPECULAR,
            "texture_specular",
            textures
        );

        loadMaterialTextures(
            material,
            aiTextureType_HEIGHT,
            "texture_normal",
            textures
        );

        loadMaterialTextures(
            material,
            aiTextureType_AMBIENT,
            "texture_height",
            textures
        );
    }

    return Mesh(vertices, indices, textures);
}

void Model::loadMaterialTextures(
    aiMaterial* material,
    aiTextureType type,
    const std::string& typeName,
    std::vector<Texture>& textures
)
{
    for (unsigned int i = 0;
        i < material->GetTextureCount(type);
        ++i)
    {
        aiString str;

        if (material->GetTexture(type, i, &str) != AI_SUCCESS)
            continue;

        std::string relativePath = str.C_Str();


        // Assimp can also report paths for textures embedded in the
        // model. Embedded texture loading needs separate handling.
        if (!relativePath.empty() && relativePath[0] == '*')
        {
            std::cout << "Embedded texture not supported by this loader: "
                << relativePath << '\n';
            continue;
        }

        std::string fullPath = directory + "/" + relativePath;

        // Reuse textures that have already been loaded.
        bool found = false;

        for (const Texture& loaded : textures_loaded)
        {
            if (loaded.path == fullPath)
            {
                textures.push_back(loaded);
                found = true;
                break;
            }
        }

        if (found)
            continue;

        Texture texture;

        texture.type = typeName;
        texture.path = fullPath;

        texture.setTextureParameter_XY(GL_REPEAT);
        texture.setTextureParameter_filter(GL_LINEAR);

        bool loaded = texture.loadImage(fullPath.c_str());



        if (!loaded)
            continue;

        textures.push_back(texture);
        textures_loaded.push_back(texture);
    }
}

void Model::free()
{
    for (Mesh& mesh : meshes) mesh.free();
    for (Texture& t : textures_loaded) t.free();
}