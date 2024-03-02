#pragma once
#include "../GameComponent.h"

#include <SimpleMath.h>

class SunSystemGame;
class SphereComponent;

class SunComponent : public GameComponent
{
private:
    SunSystemGame& game_;

    SphereComponent* sphere_;

    DirectX::SimpleMath::Vector3 position_;
    DirectX::SimpleMath::Vector3 scale_;
public:
    SunComponent(SunSystemGame& game, DirectX::SimpleMath::Vector3 position);
    ~SunComponent();

    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void Reload() override;
    virtual void DestroyResources() override;
};

