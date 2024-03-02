#pragma once
#include "../GameComponent.h"

class PlanetComponent;

class SatelliteComponent : public GameComponent
{
private:
    PlanetComponent& parent_;
public:
    SatelliteComponent(PlanetComponent& parent);
    ~SatelliteComponent();

    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void Reload() override;
    virtual void DestroyResources() override;
};

