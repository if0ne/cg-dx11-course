#include "AssetLoader.h"

#include <SimpleMath.h>

#include "MeshComponent.h"

Texture LoadMaterialTextures(const std::string& directory, aiMaterial* mat, aiTextureType type) {
    aiString str;
    mat->Get(AI_MATKEY_TEXTURE(type, 0), str);
    
    if (str.length == 0) {
        return Texture{
            directory + "/default.jpg"
        };
    } else {
        return Texture{
            directory + "/" + str.C_Str()
        };
    }
}

MeshComponent* ProcessMesh(const std::string& directory, aiMesh* mesh, const aiScene* scene, DirectX::BoundingBox& localAABB) {
    std::vector<Vertex> vertices;
    std::vector<int> indices;

    for (int i = 0; i < mesh->mNumVertices; i++) {
        Vertex v{
            DirectX::SimpleMath::Vector3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
            DirectX::SimpleMath::Vector3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z),
            DirectX::SimpleMath::Vector2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
        };

        vertices.push_back(std::move(v));
    }

    for (int i = 0; i < mesh->mNumFaces; i++) {
        const aiFace face = mesh->mFaces[i];

        for (int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }

    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    Texture texturePath = LoadMaterialTextures(directory, material, aiTextureType_DIFFUSE);

    DirectX::BoundingBox meshAABB;
    DirectX::SimpleMath::Vector3 notTransMin = { mesh->mAABB.mMin.x,mesh->mAABB.mMin.y,mesh->mAABB.mMin.z };
    DirectX::SimpleMath::Vector3 notTransMax = { mesh->mAABB.mMax.x,mesh->mAABB.mMax.y,mesh->mAABB.mMax.z };

    auto hDiag = (notTransMax - notTransMin) / 2.f;

    meshAABB.Center = (notTransMax + notTransMin) / 2.f;

    meshAABB.Extents.x = std::abs(hDiag.x);
    meshAABB.Extents.y = std::abs(hDiag.y);
    meshAABB.Extents.z = std::abs(hDiag.z);


    DirectX::BoundingBox::CreateMerged(localAABB, localAABB, meshAABB);

    return new MeshComponent(std::move(vertices), std::move(indices), std::move(texturePath));
}

std::vector<MeshComponent*> ProcessNode(const std::string& directory, aiNode* node, const aiScene* scene, DirectX::BoundingBox& localAABB) {
    std::vector<MeshComponent*> returned;

    for (int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        returned.push_back(ProcessMesh(directory, mesh, scene, localAABB));
    }

    for (int i = 0; i < node->mNumChildren; i++) {
        auto meshes = ProcessNode(directory, node->mChildren[i], scene, localAABB);

        for (auto&& mesh : meshes) {
            returned.push_back(std::move(mesh));
        }
    }

    return returned;
}

ModelComponent& AssetLoader::LoadModel(std::string& path) {
    auto result = loadedModels_.find(path);
    if (result != loadedModels_.end()) {
        return result->second;
    }

    auto searchPath = kDirectory + path;
    const aiScene* scene = importer_.ReadFile(
        searchPath, 
        aiProcess_Triangulate | 
        aiProcess_FlipUVs | 
        aiProcess_CalcTangentSpace | 
        aiProcess_GenBoundingBoxes
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::exception("Wrong path");
    }

    DirectX::BoundingBox localAABB{ DirectX::SimpleMath::Vector3::Zero,DirectX::SimpleMath::Vector3::Zero };
    auto meshes = ProcessNode(kDirectory, scene->mRootNode, scene, localAABB);
    auto model = ModelComponent(std::move(meshes), localAABB);

    model.Initialize();

    loadedModels_.insert(std::make_pair(path, std::move(model)));
    return loadedModels_.find(path)->second;
}
