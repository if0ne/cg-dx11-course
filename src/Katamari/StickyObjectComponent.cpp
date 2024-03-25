#include "StickyObjectComponent.h"

#include <SimpleMath.h>

#include "../Game.h"
#include "../InputDevice.h"
#include "../Camera.h"
#include "../OrbitCameraController.h"
#include "../ModelComponent.h"

#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

StickyObjectComponent::StickyObjectComponent(ModelComponent* gfx) : GameComponent() {

}

StickyObjectComponent::~StickyObjectComponent() {

}

void StickyObjectComponent::Initialize() {
    
}

void StickyObjectComponent::Update(float deltaTime) {

}

void StickyObjectComponent::Draw() {

}

void StickyObjectComponent::Reload() {

}

void StickyObjectComponent::DestroyResources() {

}

DirectX::BoundingBox& StickyObjectComponent::GetCollision() {
    return gfx_->AABB();
}

