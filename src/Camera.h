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

    DirectX::SimpleMath::Matrix GetCameraMatrix() const;
    DirectX::SimpleMath::Vector3 GetPosition() const;
    DirectX::SimpleMath::Vector3 GetForwardVector() const;
    DirectX::SimpleMath::Vector3 GetUpVector() const;

    void SetView(DirectX::SimpleMath::Matrix view) {
        view_ = view;
    }

    void UpdateProjection();
};

