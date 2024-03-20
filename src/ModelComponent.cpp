#include "ModelComponent.h"

#include "Game.h"
#include "RenderContext.h"
#include "MeshComponent.h"

using namespace DirectX::SimpleMath;

void ModelComponent::ProcessNode(const aiNode* node, const aiScene* scene) {
    for (int i = 0; i < node->mNumMeshes; i++) {
        ModelComponent* cmp = new ModelComponent(path_);

        auto mesh = scene->mMeshes[node->mMeshes[i]];

        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex v {
                Vector3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
                Vector2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
            };

            cmp->vertices_.push_back(std::move(v));
        }

        for (int i = 0; i < mesh->mNumFaces; i++)
        {
            const aiFace face = mesh->mFaces[i];

            for (int j = 0; j < face.mNumIndices; j++) {
                cmp->indices_.push_back(face.mIndices[j]);
            } 
        }

        children_.push_back(cmp);
    }

    for (int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
}

void ModelComponent::Initialize() {
    for (auto& child : children_) {
        child.Initialize();
    }
}

void ModelComponent::Update(float deltaTime) {
}

void ModelComponent::Draw() {
    for (auto& child : children_) {
        child.Draw();
    }
}

void ModelComponent::Reload() {
    for (auto& child : children_) {
        child.Reload();
    }
}

void ModelComponent::DestroyResources() {
    for (auto& child : children_) {
        child.DestroyResources();
    }
}
