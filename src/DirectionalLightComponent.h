#pragma once
#include "GameComponent.h"

#include <SimpleMath.h>

struct DirectionalLightData {
    DirectX::SimpleMath::Vector4 direction;
    DirectX::SimpleMath::Vector4 color;
};

class DirectionalLightComponent : public GameComponent
{
private:
    DirectX::SimpleMath::Vector3 direction_;
    DirectX::SimpleMath::Vector3 color_;
    float intensity_;
public:
    DirectionalLightComponent(
        DirectX::SimpleMath::Vector3 direction, 
        DirectX::SimpleMath::Vector3 color,
        float intensity
    ) : 
        direction_(direction),
        color_(color),
        intensity_(intensity),
        GameComponent() {
    }

    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void Reload();
    virtual void DestroyResources();

    DirectionalLightData RenderData() {
        return DirectionalLightData{
            DirectX::SimpleMath::Vector4(direction_.x, direction_.y, direction_.z, 0.0),
            DirectX::SimpleMath::Vector4(color_.x, color_.y, color_.z, intensity_),
        };
    }
};

