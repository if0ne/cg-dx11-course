#pragma once
#include "../GameComponent.h"

#include <SimpleMath.h>
#include <vector>

class SunSystemGame;
//class SatelliteComponent;

class SphereComponent;

class PlanetComponent : public GameComponent
{
private:
    SunSystemGame& game_;
    //std::vector<SatelliteComponent> satellites_;

    SphereComponent* sphere_;
    
    DirectX::SimpleMath::Vector3 position_;
    DirectX::SimpleMath::Vector3 scale_;

    DirectX::SimpleMath::Vector3 rotation_;

    float angle_;
    float rotationSpeed_;
public:
    PlanetComponent(SunSystemGame& game, DirectX::SimpleMath::Vector3 position);
    ~PlanetComponent();

    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void Reload() override;
    virtual void DestroyResources() override;
};

