#pragma once
#include "../GameComponent.h"

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <SimpleMath.h>

#include <string>
#include <vector>

#include "../ModelComponent.h"

class PlayerComponent;
class KatamariGame;

class StickyObjectComponent : public GameComponent
{
private:
    KatamariGame& game_;

    DirectX::SimpleMath::Vector3 position_;

    PlayerComponent* parent_;
    ModelComponent* gfx_;
    Material mat_;

    std::string path_;
    float scale_;
public:
    StickyObjectComponent(std::string path, float scale, DirectX::SimpleMath::Vector3 position, Material mat, KatamariGame& game);
    ~StickyObjectComponent();

    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void Reload() override;
    virtual void DestroyResources() override;

    DirectX::BoundingBox GetCollision();

    Material GetMaterial() {
        return mat_;
    }
};
