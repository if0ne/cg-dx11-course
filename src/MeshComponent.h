#pragma once
#include <string>
#include <vector>
#include <SimpleMath.h>
#include <d3d.h>
#include <d3d11.h>

#include "GameComponent.h"

class ModelComponent;

struct Texture {
    std::string diff;
    std::string normal;
};  

struct Vertex {
    DirectX::SimpleMath::Vector3 Position;
    DirectX::SimpleMath::Vector3 Normal;
    DirectX::SimpleMath::Vector2 UV;
    DirectX::SimpleMath::Vector3 Tangent;
};

class MeshComponent : public GameComponent {
private:
    std::vector<Vertex> vertices_;
    std::vector<int> indices_;
    Texture texturePath_;

    ID3D11Buffer* vb_;
    ID3D11Buffer* ib_;

    ID3D11ShaderResourceView* texture_;
    ID3D11Resource* textureData_;
    ID3D11SamplerState* sampler_;

    ID3D11ShaderResourceView* normal_;
    ID3D11Resource* normalData_;
public:
    MeshComponent(std::vector<Vertex>&& vertices, std::vector<int>&& indices, Texture&& textures) :
        vertices_(vertices),
        indices_(indices),
        texturePath_(textures),
        GameComponent()
    {
        vb_ = nullptr;
        ib_ = nullptr;
        texture_ = nullptr;
        sampler_ = nullptr;
    }

    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void Reload();
    virtual void DestroyResources();

    friend class ModelComponent;
};
