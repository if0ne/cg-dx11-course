#pragma once
#include "GameComponent.h"

#include <SimpleMath.h>
#include <d3d.h>
#include <d3d11.h>

class CubeComponent : public GameComponent
{
private:
    DirectX::SimpleMath::Vector3 vertices_[16];
    int indices_[36];

    ID3D11Buffer* vb_;
    ID3D11Buffer* ib_;
public:
    CubeComponent() : GameComponent() {
        vb_ = nullptr;
        ib_ = nullptr;
    }

    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void Reload();
    virtual void DestroyResources();
};

