#pragma once
#include "../GameComponent.h"

#include <SimpleMath.h>

class PlanetComponent;
class SatelliteComponent;
class CubeComponent;
class SunSystemGame;

class SatelliteComponent : public GameComponent
{
private:
    PlanetComponent& parent_;
    SunSystemGame& game_;

    CubeComponent* cube_;

    DirectX::SimpleMath::Vector3 localPosition_;
    DirectX::SimpleMath::Vector3 globalPosition_;

    float scale_;

    DirectX::SimpleMath::Vector3 globalAxisRotation_;

    float globalAngle_;
    float globalRotationSpeed_;

public:
    SatelliteComponent(PlanetComponent& parent, SunSystemGame& game, float radius, float size);
    ~SatelliteComponent();

    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void Reload() override;
    virtual void DestroyResources() override;
};

