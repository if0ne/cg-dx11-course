#pragma once
#include "GameComponent.h"

#include <SimpleMath.h>

struct AmbientLightData {
    DirectX::SimpleMath::Vector3 color;
    float intensity;
};

class AmbientLightComponent :
    public GameComponent
{
private:
    DirectX::SimpleMath::Vector3 color_;
    float intensity_;
public:
    AmbientLightComponent(
        DirectX::SimpleMath::Vector3 color,
        float intensity
    ) :
        color_(color),
        intensity_(intensity),
        GameComponent() {
    }

    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void Reload();
    virtual void DestroyResources();

    AmbientLightData RenderData() {
        return AmbientLightData{
            color_, 
            intensity_
        };
    }
};

