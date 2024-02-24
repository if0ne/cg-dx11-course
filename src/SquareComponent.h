#pragma once
#include "GameComponent.h"

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

class Game;

struct ConstData {
    DirectX::XMFLOAT4 offset;
};

class SquareComponent : public GameComponent
{
private:
    float x_;
    float y_;
    float speed_;

    ID3D11InputLayout* layout_;
    ID3D11VertexShader* vertexShader_;
    ID3D11PixelShader* pixelShader_;

    ID3DBlob* vertexBC_;
    ID3DBlob* pixelBC_;

    ID3D11RasterizerState* rastState_;

    ID3D11Buffer* vb_;
    ID3D11Buffer* ib_;
    ID3D11Buffer* constBuffer_;

    DirectX::XMFLOAT4 points_[8];
public:
    SquareComponent(float offset);

    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void Reload();
    virtual void DestroyResources();
};

