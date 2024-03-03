#pragma once

#include <SimpleMath.h>

#include "InputDevice.h"

class Camera;
class Game;

class CameraController {
protected:
    Game& game_;

    float yaw_;
    float pitch_;

    float sensitivity_;

    Camera& camera_;
    DirectX::SimpleMath::Vector3 cameraPosition_;

    bool isActive_;
public:
    CameraController(Camera& camera);

    virtual void Update(float deltaTime) = 0;
    virtual void OnMouseMove(const MouseMoveEventArgs& args) = 0;

    void Active(bool active) {
        isActive_ = active;
    }
};

