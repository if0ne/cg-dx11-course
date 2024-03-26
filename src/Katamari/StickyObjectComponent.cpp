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

StickyObjectComponent::StickyObjectComponent(std::string path, float scale, Vector3 position, KatamariGame& game) :
    game_(game), 
    scale_(scale),
    path_(path), 
    position_(position),
    GameComponent() 
{
    parent_ = nullptr;
}

StickyObjectComponent::~StickyObjectComponent() {

}

void StickyObjectComponent::Initialize() {
    gfx_ = ctx_.GetAssetLoader().LoadModel(path_);
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
    auto scale = Matrix::CreateScale(scale_);
    if (parent_) {
        auto rotation = parent_->Rotation();
        auto rotMat = Matrix::CreateFromQuaternion(rotation);
        auto translation = Vector3::Transform(position_, rotation) + parent_->Position();
        auto matrix = scale * rotMat * Matrix::CreateTranslation(translation);

        game_.UpdateModelBuffer(matrix);
    } else {
        auto translation = Matrix::CreateTranslation(position_);
        auto matrix = scale * translation;

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
    aabb.Extents.x *= scale_;
    aabb.Extents.y *= scale_;
    aabb.Extents.z *= scale_;
    return aabb;
}

