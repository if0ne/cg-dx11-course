#include "FreeCameraController.h"

#include "Game.h"
#include "Camera.h"
#include "InputDevice.h"
#include "Keys.h"

using namespace DirectX::SimpleMath;

FreeCameraController::FreeCameraController(Camera& camera) : CameraController(camera) {
    velocity_ = 100.0f;

    game_.GetInputDevice().AddMouseMoveListener([this](auto& args) {
        OnMouseMove(args);
    });
}

void FreeCameraController::Update(float deltaTime) {
    auto input = game_.GetInputDevice();

    auto rotMat = Matrix::CreateFromYawPitchRoll(yaw_, pitch_, 0);

    auto dir = Vector3::Zero;

    if (input.IsKeyDown(Keys::W)) dir += Vector3(1.0, 0.0, 0.0);
    if (input.IsKeyDown(Keys::S)) dir += Vector3(-1.0, 0.0, 0.0);
    if (input.IsKeyDown(Keys::D)) dir += Vector3(0.0, 0.0, 1.0);
    if (input.IsKeyDown(Keys::A)) dir += Vector3(0.0, 0.0, -1.0);

    if (input.IsKeyDown(Keys::Space))       dir += Vector3(0.0, 1.0, 0.0);
    if (input.IsKeyDown(Keys::LeftControl)) dir += Vector3(0.0, -1.0, 0.0);

    dir.Normalize();

    auto direction = rotMat.Forward() * dir.x + Vector3::Up * dir.y + rotMat.Right() * dir.z;

    if (direction.Length() != 0) {
        direction.Normalize();
    }

    cameraPosition_ = cameraPosition_ + direction * velocity_ * deltaTime;
    
    camera_.SetView(Matrix::CreateLookAt(cameraPosition_, cameraPosition_ + rotMat.Forward(), rotMat.Up()));
}

void FreeCameraController::OnMouseMove(const MouseMoveEventArgs& args) {
    if (game_.GetInputDevice().IsKeyDown(Keys::LeftShift)) return;

    yaw_ -= args.Offset.x * 0.003f * sensitivity_;
    pitch_ -= args.Offset.y * 0.003f * sensitivity_;

    if (args.WheelDelta > 0) velocity_ *= 1.1f;
    if (args.WheelDelta < 0) velocity_ *= 0.9f;
}
