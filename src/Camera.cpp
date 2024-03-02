#include "Camera.h"

#include "Game.h"
#include "Window.h"

Camera::Camera() : game_(Game::GetSingleton()) {
    farPlane_ = 10000.0f;
    nearPlane_ = 0.1f;
    fov_ = 90.0 * M_PI / 180.0;

    view_ = DirectX::SimpleMath::Matrix::Identity;

    UpdateProjection();
}

DirectX::SimpleMath::Matrix Camera::GetCameraMatrix() const {
    return view_ * proj_;
}

DirectX::SimpleMath::Vector3 Camera::GetPosition() const {
    DirectX::SimpleMath::Matrix inv;
    view_.Invert(inv);
    return inv.Translation();
}

DirectX::SimpleMath::Vector3 Camera::GetForwardVector() const {
    DirectX::SimpleMath::Matrix inv;
    view_.Invert(inv);
    return inv.Forward();
}

DirectX::SimpleMath::Vector3 Camera::GetUpVector() const {
    DirectX::SimpleMath::Matrix inv;
    view_.Invert(inv);
    return inv.Up();
}

void Camera::UpdateProjection() {
    proj_ = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
        fov_,
        game_.GetWindow().GetAspectRatio(),
        nearPlane_,
        farPlane_
    );
}
