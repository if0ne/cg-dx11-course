#include "RacketComponent.h"

#include "../Game.h"
#include "../InputDevice.h"
#include "../Keys.h"
#include "../RenderContext.h"
#include "../Window.h"

#include "RenderMisc.h"

RacketComponent::RacketComponent(float x, float y, Keys upKey, Keys downKey) : GameComponent() {
    points_[0] = DirectX::XMFLOAT4(0.00f, 0.00f, 0.0f, 1.0f);
    points_[1] = DirectX::XMFLOAT4(0.05f, 0.0f, 0.0f, 1.0f);
    points_[2] = DirectX::XMFLOAT4(0.05f, -0.2f, 0.0f, 1.0f);
    points_[3] = DirectX::XMFLOAT4(0.00f, -0.2f, 0.0f, 1.0f);

    x_ = x;
    y_ = y;
    speed_ = 1.0;

    upKey_ = upKey;
    downKey_ = downKey;
}

void RacketComponent::Initialize() {
    D3DCompileFromFile(
        L"./shaders/PingPong.hlsl",
        nullptr,
        nullptr,
        "VSMain",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &vertexBC_,
        nullptr
    );

    D3DCompileFromFile(
        L"./shaders/PingPong.hlsl",
        nullptr,
        nullptr,
        "PSMain",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &pixelBC_,
        nullptr
    );

    ctx_.GetRenderContext().GetDevice()->CreateVertexShader(
        vertexBC_->GetBufferPointer(),
        vertexBC_->GetBufferSize(),
        nullptr, &vertexShader_
    );

    ctx_.GetRenderContext().GetDevice()->CreatePixelShader(
        pixelBC_->GetBufferPointer(),
        pixelBC_->GetBufferSize(),
        nullptr, &pixelShader_
    );

    D3D11_INPUT_ELEMENT_DESC inputElements[] = {
        D3D11_INPUT_ELEMENT_DESC {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        }
    };

    ctx_.GetRenderContext().GetDevice()->CreateInputLayout(
        inputElements,
        1,
        vertexBC_->GetBufferPointer(),
        vertexBC_->GetBufferSize(),
        &layout_
    );

    D3D11_BUFFER_DESC vertexBufDesc = {};
    vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufDesc.CPUAccessFlags = 0;
    vertexBufDesc.MiscFlags = 0;
    vertexBufDesc.StructureByteStride = 0;
    vertexBufDesc.ByteWidth = sizeof(DirectX::XMFLOAT4) * std::size(points_);

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = points_;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&vertexBufDesc, &vertexData, &vb_);

    int indeces[] = { 0,1,3, 1,2,3 };
    D3D11_BUFFER_DESC indexBufDesc = {};
    indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufDesc.CPUAccessFlags = 0;
    indexBufDesc.MiscFlags = 0;
    indexBufDesc.StructureByteStride = 0;
    indexBufDesc.ByteWidth = sizeof(int) * std::size(indeces);

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indeces;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&indexBufDesc, &indexData, &ib_);

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID;

    ctx_.GetRenderContext().GetDevice()->CreateRasterizerState(&rastDesc, &rastState_);

    D3D11_BUFFER_DESC constBufDesc = {};
    constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
    constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constBufDesc.MiscFlags = 0;
    constBufDesc.StructureByteStride = 0;
    constBufDesc.ByteWidth = sizeof(ConstantData);

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&constBufDesc, nullptr, &constBuffer_);
}

void RacketComponent::Update(float deltaTime) {
    if (ctx_.GetInputDevice().IsKeyDown(upKey_)) {
        y_ += speed_ * deltaTime;
    }

    if (ctx_.GetInputDevice().IsKeyDown(downKey_)) {
        y_ -= speed_ * deltaTime;
    }
}

void RacketComponent::Draw() {
    UINT strides[] = { sizeof(DirectX::XMFLOAT4) };
    UINT offsets[] = { 0 };

    ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
    ctx_.GetRenderContext().GetContext()->IASetInputLayout(layout_);
    ctx_.GetRenderContext().GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(ib_, DXGI_FORMAT_R32_UINT, 0);
    ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &vb_, strides, offsets);
    ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(0, 1, &constBuffer_);

    D3D11_MAPPED_SUBRESOURCE res = {};
    ctx_.GetRenderContext().GetContext()->Map(constBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
    ConstantData data = {};
    data.v = { x_, y_, 0.0, 0.0 };
    data.aspect = { ctx_.GetWindow().GetAspectRatio(), 0.0, 0.0, 0.0 };

    memcpy(res.pData, &data, sizeof(ConstantData));
    ctx_.GetRenderContext().GetContext()->Unmap(constBuffer_, 0);

    ctx_.GetRenderContext().GetContext()->DrawIndexed(6, 0, 0);
}

void RacketComponent::Reload() {

}

void RacketComponent::DestroyResources() {
    layout_->Release();
    vertexShader_->Release();
    pixelShader_->Release();
    vertexBC_->Release();
    pixelBC_->Release();
    rastState_->Release();
    vb_->Release();
    ib_->Release();
    constBuffer_->Release();
}