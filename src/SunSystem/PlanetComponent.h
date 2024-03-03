#pragma once
#include "../GameComponent.h"

#include <SimpleMath.h>
#include <vector>

class SunSystemGame;
class SatelliteComponent;
class SphereComponent;

class PlanetComponent : public GameComponent
{
private:
    SunSystemGame& game_;
    std::vector<SatelliteComponent*> satellites_;

    SphereComponent* sphere_;
    
    DirectX::SimpleMath::Vector3 localPosition_;
    DirectX::SimpleMath::Vector3 globalPosition_;

    float scale_;

    DirectX::SimpleMath::Vector3 localAxisRotation_;

    float localAngle_;
    float localRotationSpeed_;

    float globalAngle_;
    float globalRotationSpeed_;
public:
    PlanetComponent(
        SunSystemGame& game, 
        float radius, 
        float size, 
        int satelliteCount,
        DirectX::SimpleMath::Vector3 firstColor,
        DirectX::SimpleMath::Vector3 secondColor
    );
    ~PlanetComponent();

    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void Reload() override;
    virtual void DestroyResources() override;

    DirectX::SimpleMath::Vector3 GetGlobalPosition() const {
        return globalPosition_;
    }
};

