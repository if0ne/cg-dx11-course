#include "Camera.h"

#include "Game.h"
#include "Window.h"

Camera::Camera() : game_(Game::GetSingleton()) {
    farPlane_ = 300.0f;
    nearPlane_ = 0.1f;
    fov_ = 90.0 * M_PI / 180.0;

    view_ = DirectX::SimpleMath::Matrix::Identity;

    UpdatePerspectiveProjection();
}

DirectX::SimpleMath::Matrix Camera::CameraMatrix() const {
    return view_ * proj_;
}

DirectX::SimpleMath::Vector3 Camera::Position() const {
    DirectX::SimpleMath::Matrix inv;
    view_.Invert(inv);
    return inv.Translation();
}

DirectX::SimpleMath::Vector3 Camera::ForwardVector() const {
    DirectX::SimpleMath::Matrix inv;
    view_.Invert(inv);
    return inv.Forward();
}

DirectX::SimpleMath::Vector3 Camera::UpVector() const {
    DirectX::SimpleMath::Matrix inv;
    view_.Invert(inv);
    return inv.Up();
}

DirectX::SimpleMath::Vector3 Camera::RightVector() const {
    DirectX::SimpleMath::Matrix inv;
    view_.Invert(inv);
    return inv.Right();
}

void Camera::UpdatePerspectiveProjection() {
    proj_ = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
        fov_,
        game_.GetWindow().GetAspectRatio(),
        nearPlane_,
        farPlane_
    );
}

void Camera::UpdateOrthoProjection() {
    proj_ = DirectX::SimpleMath::Matrix::CreateOrthographic(
        game_.GetWindow().GetWidth(),
        game_.GetWindow().GetHeight(),
        nearPlane_,
        farPlane_
    );
}

float Camera::AspectRatio() {
    return game_.GetWindow().GetAspectRatio();
}
