#pragma once
#include "../GameComponent.h"

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <SimpleMath.h>

#include <vector>

class Camera;
class StickyObjectComponent;
class OrbitCameraController;
class ModelComponent;
class KatamariGame;

class PlayerComponent : public GameComponent
{
private:
    KatamariGame& game_;
    Camera& camera_;
    OrbitCameraController* cameraController_;
    std::vector<StickyObjectComponent*> objects_;
    ModelComponent* gfx_;

    DirectX::SimpleMath::Vector3 position_;
    DirectX::SimpleMath::Quaternion rotation_;
public:
    PlayerComponent(Camera& camera, KatamariGame& game);
    ~PlayerComponent();

    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void Reload() override;
    virtual void DestroyResources() override;

    DirectX::BoundingBox& GetCollision();
};
