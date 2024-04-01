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

struct MeshRenderData {
    ID3D11Buffer* vb;
    ID3D11Buffer* ib;

    ID3D11ShaderResourceView* texture;
    ID3D11ShaderResourceView* normal;

    ID3D11SamplerState* sampler;
    size_t indexCount;
};

struct RenderData {
    DirectX::SimpleMath::Matrix transform;
    Material material;

    std::vector<MeshRenderData> meshData;
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

    std::vector<MeshRenderData> GetMeshRenderData();

    DirectX::BoundingBox& AABB() {
        return AABB_;
    }
};
