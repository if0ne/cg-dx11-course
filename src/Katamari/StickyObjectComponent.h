#pragma once
#include "../GameComponent.h"

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <SimpleMath.h>

#include <string>
#include <vector>

class PlayerComponent;
class ModelComponent;
class KatamariGame;

class StickyObjectComponent : public GameComponent
{
private:
    KatamariGame& game_;

    DirectX::SimpleMath::Vector3 position_;

    PlayerComponent* parent_;
    ModelComponent* gfx_;

    std::string path_;
public:
    StickyObjectComponent(std::string path, DirectX::SimpleMath::Vector3 position, KatamariGame& game);
    ~StickyObjectComponent();

    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void Reload() override;
    virtual void DestroyResources() override;

    DirectX::BoundingBox GetCollision();
};
