#pragma once
#include "CameraController.h"

#include <SimpleMath.h>

class OrbitCameraController : public CameraController {
private:
    float distance_;

    DirectX::SimpleMath::Vector3& target_;
public:
    OrbitCameraController(Camera& camera, DirectX::SimpleMath::Vector3& target);

    virtual void Update(float deltaTime) override;
    virtual void OnMouseMove(const MouseMoveEventArgs& args) override;
};

