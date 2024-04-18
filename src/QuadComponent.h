#pragma once
#include "GameComponent.h"

#include <SimpleMath.h>
#include <d3d.h>
#include <d3d11.h>
#include <vector>

class QuadComponent : public GameComponent
{
private:
    std::vector<DirectX::SimpleMath::Vector3> vertices_;
    std::vector<int> indices_;

    ID3D11Buffer* vb_;
    ID3D11Buffer* ib_;
public:
    QuadComponent() :
        GameComponent()
    {
        vb_ = nullptr;
        ib_ = nullptr;
    }

    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void Reload();
    virtual void DestroyResources();
};
