#include "StickyPointLight.h"

#include "../PointLightComponent.h"
#include "KatamariGame.h"
#include "PlayerComponent.h"

using namespace DirectX::SimpleMath;

StickyPointLight::StickyPointLight(PointLightComponent* pointLight, KatamariGame& game) :
    game_(game),
    pointLight_(pointLight),
    GameComponent()
{
    parent_ = nullptr;
    position_ = pointLight_->Position();
}

StickyPointLight::~StickyPointLight() {

}

void StickyPointLight::Initialize() {
}

void StickyPointLight::Update(float deltaTime) {
    if (parent_) {
        pointLight_->Position(position_);
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

void StickyPointLight::Draw() {
}

void StickyPointLight::Reload() {

}

void StickyPointLight::DestroyResources() {

}

DirectX::BoundingSphere StickyPointLight::GetCollision() {
    return DirectX::BoundingSphere{
        pointLight_->Position(), pointLight_->Radius() - 2.0f
    };
}
