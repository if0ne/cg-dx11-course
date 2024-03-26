#include "StickyObjectComponent.h"

#include <SimpleMath.h>

#include "../AssetLoader.h"
#include "../Game.h"
#include "../InputDevice.h"
#include "../Camera.h"
#include "../OrbitCameraController.h"
#include "../ModelComponent.h"

#include "KatamariGame.h"
#include "PlayerComponent.h"

using namespace DirectX::SimpleMath;

StickyObjectComponent::StickyObjectComponent(std::string path, Vector3 position, KatamariGame& game) : 
    game_(game), 
    path_(path), 
    position_(position),
    GameComponent() 
{
    parent_ = nullptr;
}

StickyObjectComponent::~StickyObjectComponent() {

}

void StickyObjectComponent::Initialize() {
    gfx_ = &ctx_.GetAssetLoader().LoadModel(path_);
}

void StickyObjectComponent::Update(float deltaTime) {
    if (parent_) {

    } else {
        if (GetCollision().Intersects(game_.Player().GetCollision())) {
            parent_ = &game_.Player();

            auto rotMatInv = Quaternion::Identity;
            parent_->Rotation().Inverse(rotMatInv);
            auto pos = Vector3::Transform(position_ - parent_->Position(), rotMatInv);
            position_ = pos;
        }
    }
}

void StickyObjectComponent::Draw() {
    if (parent_) {
        auto rotation = parent_->Rotation();

        auto translation = Vector3::Transform(position_, rotation) + parent_->Position();
        auto matrix = Matrix::CreateTranslation(translation);

        game_.UpdateModelBuffer(matrix);
    } else {
        auto translation = Matrix::CreateTranslation(position_);
        auto matrix = translation;

        game_.UpdateModelBuffer(matrix);
    }
    
    gfx_->Draw();
}

void StickyObjectComponent::Reload() {

}

void StickyObjectComponent::DestroyResources() {

}

DirectX::BoundingBox StickyObjectComponent::GetCollision() {
    auto aabb = gfx_->AABB();
    aabb.Center.x += position_.x;
    aabb.Center.y += position_.y;
    aabb.Center.z += position_.z;
    return aabb;
}

