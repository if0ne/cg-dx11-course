#include "PlayerComponent.h"

#include <SimpleMath.h>

#include "../AssetLoader.h"
#include "../Game.h"
#include "../InputDevice.h"
#include "../Camera.h"
#include "../OrbitCameraController.h"
#include "../ModelComponent.h"

#include "KatamariGame.h"
#include "StickyObjectComponent.h"

using namespace DirectX::SimpleMath;

PlayerComponent::PlayerComponent(Camera& camera, KatamariGame& game) : game_(game), camera_(camera), GameComponent() {
    cameraController_ = new OrbitCameraController(camera, Vector3(0.0, 0.0, 5.0), &position_);
    position_ = Vector3(0.0, 0.0, 0.0);
    rotation_ = Quaternion::Identity;
}

PlayerComponent::~PlayerComponent() {
    delete cameraController_;
}

void PlayerComponent::Initialize() {
    auto path = std::string("/rock.fbx");
    gfx_ = &ctx_.GetAssetLoader().LoadModel(path);
}

void PlayerComponent::Update(float deltaTime) {
    auto& input = ctx_.GetInputDevice();
    Vector3 dir{ 0.0, 0.0, 0.0 };

    if (input.IsKeyDown(Keys::W)) dir += camera_.ForwardVector();
    if (input.IsKeyDown(Keys::S)) dir += -camera_.ForwardVector();
    if (input.IsKeyDown(Keys::D)) dir += camera_.RightVector();
    if (input.IsKeyDown(Keys::A)) dir += -camera_.RightVector();

    ;
    dir.Normalize();

    if (dir.Length() > 0.0) {
        auto rotAxis = dir.Cross(Vector3::Up);
        auto rotQuat = Quaternion::CreateFromAxisAngle(rotAxis, -deltaTime * 5.0f);
        rotation_ *= rotQuat;

        position_ += 5.0f * deltaTime * dir;
    }

    cameraController_->Update(deltaTime);
}

void PlayerComponent::Draw() {
    auto rotation = Matrix::CreateFromQuaternion(rotation_);
    auto translation = Matrix::CreateTranslation(position_);

    auto matrix = rotation * translation;

    game_.UpdateModelBuffer(matrix);
    gfx_->Draw();
}

void PlayerComponent::Reload() {

}

void PlayerComponent::DestroyResources() {

}

DirectX::BoundingBox& PlayerComponent::GetCollision() {
    return gfx_->AABB();
}

