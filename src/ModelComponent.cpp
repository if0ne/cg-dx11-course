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
