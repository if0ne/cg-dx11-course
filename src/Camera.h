#pragma once
#define _USE_MATH_DEFINES

#include <math.h>
#include <SimpleMath.h>

class Game;

class Camera
{
private:
    Game& game_;

    DirectX::SimpleMath::Matrix view_;
    DirectX::SimpleMath::Matrix proj_;

    float farPlane_;
    float nearPlane_;
    float fov_;
public:
    Camera();

    DirectX::SimpleMath::Matrix CameraMatrix() const;
    DirectX::SimpleMath::Vector3 Position() const;
    DirectX::SimpleMath::Vector3 ForwardVector() const;
    DirectX::SimpleMath::Vector3 UpVector() const;
    DirectX::SimpleMath::Vector3 RightVector() const;

    void View(DirectX::SimpleMath::Matrix view) {
        view_ = view;
    }

    void UpdatePerspectiveProjection();
    void UpdateOrthoProjection();
};

