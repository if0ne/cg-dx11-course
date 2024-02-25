#pragma once
#include "../GameComponent.h"
#include "../Keys.h"

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

#include "DirectXCollision.h"

class PingPongGame;

class RacketComponent : public GameComponent
{
private:
    float x_;
    float y_;

    float w_;
    float h_;

    float speed_;
    float dirY_;

    PingPongGame& parent_;

    Keys upKey_;
    Keys downKey_;

    ID3D11InputLayout* layout_;
    ID3D11VertexShader* vertexShader_;
    ID3D11PixelShader* pixelShader_;

    ID3DBlob* vertexBC_;
    ID3DBlob* pixelBC_;

    ID3D11RasterizerState* rastState_;

    ID3D11Buffer* vb_;
    ID3D11Buffer* ib_;
    ID3D11Buffer* constBuffer_;

    DirectX::XMFLOAT4 points_[4];

public:
    RacketComponent(float x, float y, Keys upKey, Keys downKey, PingPongGame& parent);

    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void Reload();
    virtual void DestroyResources();

    DirectX::BoundingBox GetNextBoundingBox();
    DirectX::BoundingBox GetBoundingBox();
};

