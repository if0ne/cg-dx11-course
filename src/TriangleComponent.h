#pragma once
#include "GameComponent.h"

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

class Game;

class TriangleComponent : public GameComponent
{
private:
    ID3D11InputLayout* layout_;
    ID3D11VertexShader* vertexShader_;
    ID3D11PixelShader* pixelShader_;

    ID3DBlob* vertexBC_;
    ID3DBlob* pixelBC_;

    ID3D11RasterizerState* rastState_;

    ID3D11Buffer* vb_;
    ID3D11Buffer* ib_;

    DirectX::XMFLOAT4 points_[8];
public:
    TriangleComponent(Game& ctx);

    virtual void Initialize();
    virtual void Update();
    virtual void Draw();
    virtual void Reload();
    virtual void DestroyResources();
};

