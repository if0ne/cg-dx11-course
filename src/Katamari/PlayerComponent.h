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

class PlayerComponent : public GameComponent
{
private:
    OrbitCameraController* cameraController_;
    std::vector<StickyObjectComponent*> objects_;
    SphereComponent* gfx_;

    SimpleMath::Vector3 position_;
    SimpleMath::Vector3 rotation_;
public:
    PlayerComponent(Camera& camera);
    ~PlayerComponent();

    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void Reload() override;
    virtual void DestroyResources() override;
};
