#include "CameraController.h"

#include "Game.h"
#include "Camera.h"

CameraController::CameraController(Camera& camera) : 
    game_(Game::GetSingleton()),
    camera_(camera)
{
    sensitivity_ = 0.5;
}
