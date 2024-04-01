#include "ModelComponent.h"

#include "Game.h"
#include "RenderContext.h"
#include "MeshComponent.h"

using namespace DirectX::SimpleMath;

void ModelComponent::Initialize() {
    for (auto& child : children_) {
        child->Initialize();
    }
}

void ModelComponent::Update(float deltaTime) {
}

void ModelComponent::Draw() {
    for (auto& child : children_) {
        child->Draw();
    }
}

void ModelComponent::Reload() {
    for (auto& child : children_) {
        child->Reload();
    }
}

void ModelComponent::DestroyResources() {
    for (auto& child : children_) {
        child->DestroyResources();
    }
}

std::vector<MeshRenderData> ModelComponent::GetMeshRenderData() {
    std::vector<MeshRenderData> data;

    for (auto& mesh : children_) {
        data.push_back(MeshRenderData{
            mesh->vb_,
            mesh->ib_,
            mesh->texture_,
            mesh->normal_,
            mesh->sampler_,
            mesh->indices_.size()
        });
    }

    return data;
}