#pragma once
#include "../GameComponent.h"

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <SimpleMath.h>

class PointLightComponent;
class PlayerComponent;
class KatamariGame;

class StickyPointLight : public GameComponent
{
private:
    PointLightComponent* pointLight_;
    PlayerComponent* parent_;
    KatamariGame& game_;

    DirectX::SimpleMath::Vector3 position_;
public:
    StickyPointLight(PointLightComponent* pointLight, KatamariGame& game);
    ~StickyPointLight();

    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void Reload() override;
    virtual void DestroyResources() override;

    DirectX::BoundingSphere GetCollision();

    friend class KatamariPointLightPass;
};

