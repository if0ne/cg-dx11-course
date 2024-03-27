#pragma once
#include "GameComponent.h"

#include <SimpleMath.h>
#include <d3d.h>
#include <d3d11.h>

#include <vector>
#include <string>

class MeshComponent;

struct Material
{
    DirectX::SimpleMath::Vector4 baseColor;
    float reflection;
    float absorption;
    float shininess;
    float _padding;

    static Material Default() {
        return Material{
            DirectX::SimpleMath::Vector4(1.0, 1.0, 1.0, 1.0),
            1.0,
            1.0,
            1.0,
            0.0
        };
    }
};

class ModelComponent : public GameComponent
{
private:
    std::vector<MeshComponent*> children_;
    DirectX::BoundingBox AABB_;
public:
    ModelComponent(std::vector<MeshComponent*>&& children, DirectX::BoundingBox AABB) : children_(children), AABB_(AABB), GameComponent() {}

    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void Reload();
    virtual void DestroyResources();

    DirectX::BoundingBox& AABB() {
        return AABB_;
    }
};
