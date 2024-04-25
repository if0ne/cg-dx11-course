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
}

StickyPointLight::~StickyPointLight() {

}

void StickyPointLight::Initialize() {
}

void StickyPointLight::Update(float deltaTime) {
    if (parent_) {
        pointLight_->Position(parent_->Position());
    }
    else {
        if (GetCollision().Intersects(game_.Player().GetCollision())) {
            parent_ = &game_.Player();
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
        pointLight_->Position(), pointLight_->Radius()
    };
}
