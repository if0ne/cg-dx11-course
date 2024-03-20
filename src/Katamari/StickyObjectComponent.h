#pragma once
#include "../GameComponent.h"

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <SimpleMath.h>

#include <vector>

class PlayerComponent;

class StickyObjectComponent : public GameComponent
{
private:
    SimpleMath::Vector3 position_;
    SimpleMath::Vector3 rotation_;

    PlayerComponent* parent_;
    ModelComponent* gfx_;
public:
    StickyObjectComponent(ModelComponent* gfx);
    ~StickyObjectComponent();

    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void Reload() override;
    virtual void DestroyResources() override;
};
