#pragma once
#include "CameraController.h"

class Camera;

class FreeCameraController :  public CameraController {
private:
    float velocity_;

public:
    FreeCameraController(Camera& camera);

    virtual void Update(float deltaTime) override;
    virtual void OnMouseMove(const MouseMoveEventArgs& args) override;
};

