#include "OrbitCamera.h"

#include "Game.h"
#include "Camera.h"
#include "InputDevice.h"
#include "Keys.h"

using namespace DirectX::SimpleMath;

OrbitCamera::OrbitCamera(Camera& camera, DirectX::SimpleMath::Vector3& target) : CameraController(camera), target_(target) {
    distance_ = 10.0f;

    game_.GetInputDevice().AddMouseMoveListener([this](auto& args) {
        OnMouseMove(args);
    });
}

void OrbitCamera::Update(float deltaTime) {
    auto& input = game_.GetInputDevice();

    auto rotQuat = Quaternion::CreateFromYawPitchRoll(yaw_, pitch_, 0.0);
    auto rotMat = Matrix::CreateFromQuaternion(rotQuat);

    cameraPosition_ = target_ - rotMat.Backward() * distance_;
    
    camera_.SetView(Matrix::CreateLookAt(cameraPosition_, target_, rotMat.Up()));
}

void OrbitCamera::OnMouseMove(const MouseMoveEventArgs& args) {
    if (game_.GetInputDevice().IsKeyDown(Keys::LeftShift)) return;

    yaw_ -= args.Offset.x * 0.003f * sensitivity_;
    pitch_ -= args.Offset.y * 0.003f * sensitivity_;

    if (args.WheelDelta > 0) distance_ *= 1.1f;
    if (args.WheelDelta < 0) distance_ *= 0.9f;
}