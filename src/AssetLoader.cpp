#include "AssetLoader.h"
#include "ModelComponent.h"
#include "MeshComponent.h"

using namespace DirectX::SimpleMath;

std::vector<MeshComponent> ProcessNode(aiNode* node, const aiScene* scene) {
    std::vector<MeshComponent> meshes;

    for (int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(ProcessMesh(mesh, scene));
    }

    for (int i = 0; i < node->mNumChildren; i++) {
        meshes.push_back(ProcessNode(node->mChildren[i], scene));
    }

    return meshes;
}

MeshComponent ProcessMesh(aiMesh*, const aiScene* scene) {
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    for (int i = 0; i < mesh->mNumVertices; i++) {
        Vertex v {
            Vector3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
            Vector3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z),
            Vector2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
        };

        vertices.push_back(std::move(v));
    }

    for (int i = 0; i < mesh->mNumFaces; i++) {
        const aiFace face = mesh->mFaces[i];

        for (int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        } 

    }

    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        vector<Texture> loaded_texture = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), loaded_texture.begin(), loaded_texture.end());
    }

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> LoadMaterialTextures(aiMaterial *mat, aiTextureType type) {
    vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        Texture texture;
        //TODO: Texture load
        texture.id = TextureFromFile(str.C_Str(), directory);
        textures.push_back(texture);
    }
    return textures;
} 

ModelComponent& AssetLoader::LoadModel(std::string& path) {
    auto result = loadedModels_.find(path);
    if (result != loadedModels_.end()) {
        return result->second;
    }

    const aiScene* scene = importer_.ReadFile(path)

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw Exception("Wrong path");
    }

    auto meshes = ProcessNode(scene->mRootNode, scene);
    auto model = ModelComponent(meshes);

    model.Initialize();

    loadedModels_.push_back(path, model);
    return loadedModels_.find(path)->second;
}
