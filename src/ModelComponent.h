#pragma once
#include "GameComponent.h"

#include <assimp/scene.h>

#include <SimpleMath.h>
#include <d3d.h>
#include <d3d11.h>

#include <vector>
#include <string>

struct Vertex {
    DirectX::SimpleMath::Vector3 Position;
    DirectX::SimpleMath::Vector2 UV;
};

class ModelComponent : public GameComponent
{
private:
    std::vector<Vertex> vertices_;
    std::vector<int> indices_;

    std::vector<ModelComponent*> children_;

    ID3D11Buffer* vb_;
    ID3D11Buffer* ib_;

    ID3D11ShaderResourceView* texture_;
    ID3D11SamplerState* sampler_;

    std::string& path_;

    void ProcessNode(const aiNode* node, const aiScene* scene);
public:
    ModelComponent(std::string& path) : path_(path), GameComponent() {
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
};

