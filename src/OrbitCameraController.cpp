#include "OrbitCameraController.h"

#include "Game.h"
#include "Camera.h"
#include "InputDevice.h"
#include "Keys.h"

using namespace DirectX::SimpleMath;

OrbitCameraController::OrbitCameraController(Camera& camera, DirectX::SimpleMath::Vector3& target) : CameraController(camera), target_(target) {
    distance_ = 10.0f;
    cameraPosition_ = Vector3(0.0, 0.0, distance_);
    yaw_ = 0.0;
    pitch_ = 0.0;

    game_.GetInputDevice().AddMouseMoveListener([this](auto& args) {
        OnMouseMove(args);
    });
}

void OrbitCameraController::Update(float deltaTime) {
    cameraPosition_ = Vector3(0.0, 0.0, distance_);

    auto rotQuat = Quaternion::CreateFromYawPitchRoll(-yaw_, pitch_, 0.0f);
    auto rotMat = Matrix::CreateFromQuaternion(rotQuat);
    auto newPos = Vector3::Transform(cameraPosition_, rotQuat);

    camera_.SetView(Matrix::CreateLookAt(newPos, target_, rotMat.Up()));
}

void OrbitCameraController::OnMouseMove(const MouseMoveEventArgs& args) {
    if (!isActive_) { return; }

    if (game_.GetInputDevice().IsKeyDown(Keys::LeftShift)) return;

    yaw_ -= args.Offset.x * 0.003f * sensitivity_;
    pitch_ -= args.Offset.y * 0.003f * sensitivity_;

    if (args.WheelDelta > 0) distance_ *= 1.1f;
    if (args.WheelDelta < 0) distance_ *= 0.9f;
}