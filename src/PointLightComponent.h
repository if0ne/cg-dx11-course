#pragma once
#include "GameComponent.h"

#include <SimpleMath.h>

struct PointLightData {
    DirectX::SimpleMath::Vector4 position;
    DirectX::SimpleMath::Vector4 color;
};

class PointLightComponent : public GameComponent
{
private:
    DirectX::SimpleMath::Vector3 position_;
    DirectX::SimpleMath::Vector3 color_;
    float intensity_;
    float radius_;
public:
    PointLightComponent(
        DirectX::SimpleMath::Vector3 position,
        float radius,
        DirectX::SimpleMath::Vector3 color,
        float intensity
    ) :
        position_(position),
        radius_(radius),
        color_(color),
        intensity_(intensity),
        GameComponent() {
    }

    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void Reload();
    virtual void DestroyResources();

    PointLightData RenderData() {
        return PointLightData{
            DirectX::SimpleMath::Vector4(position_.x, position_.y, position_.z, radius_),
            DirectX::SimpleMath::Vector4(color_.x, color_.y, color_.z, intensity_),
        };
    }

    bool IsIntersect(DirectX::SimpleMath::Vector3 point) {
        auto bound = DirectX::BoundingSphere(position_, radius_);

        return bound.Contains(point) == DirectX::ContainmentType::CONTAINS;
    }

    DirectX::SimpleMath::Vector3 Position() {
        return position_;
    }

    void Position(DirectX::SimpleMath::Vector3 position) {
        position_ = position;
    }

    float Radius() {
        return radius_;
    }
};

