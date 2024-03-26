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
    scale_ = 1.0;//0.005;
}

PlayerComponent::~PlayerComponent() {
    delete cameraController_;
}

void PlayerComponent::Initialize() {
    auto path = std::string("/bball.fbx");
    gfx_ = ctx_.GetAssetLoader().LoadModel(path);
}

void PlayerComponent::Update(float deltaTime) {
    auto& input = ctx_.GetInputDevice();
    Vector3 dir{ 0.0, 0.0, 0.0 };

    auto cameraDir = Vector3 { camera_.ForwardVector().x, 0, camera_.ForwardVector().z };
    cameraDir.Normalize();

    if (input.IsKeyDown(Keys::W)) dir += cameraDir;
    if (input.IsKeyDown(Keys::S)) dir -= cameraDir;
    if (input.IsKeyDown(Keys::D)) dir += camera_.RightVector();
    if (input.IsKeyDown(Keys::A)) dir += -camera_.RightVector();

    dir.Normalize();

    if (dir.Length() > 0.0) {
        auto rotAxis = dir.Cross(Vector3::Up);

        auto sign = cameraDir.Cross(dir);

        auto rotQuat = Quaternion::CreateFromAxisAngle(rotAxis, -deltaTime * 5.0f);
        rotation_ *= rotQuat;

        position_ += 5.0f * deltaTime * dir;
    }

    cameraController_->Update(deltaTime);
}

void PlayerComponent::Draw() {
    auto scale = Matrix::CreateScale(scale_);
    auto rotation = Matrix::CreateFromQuaternion(rotation_);
    auto translation = Matrix::CreateTranslation(position_);

     auto matrix = scale * rotation * translation;

    game_.UpdateModelBuffer(matrix);
    gfx_->Draw();
}

void PlayerComponent::Reload() {

}

void PlayerComponent::DestroyResources() {

}

DirectX::BoundingBox PlayerComponent::GetCollision() {
    auto aabb = gfx_->AABB();
    aabb.Center.x += position_.x;
    aabb.Center.y += position_.y;
    aabb.Center.z += position_.z;
    aabb.Extents.x *= scale_;
    aabb.Extents.y *= scale_;
    aabb.Extents.z *= scale_;
    return aabb;
}

