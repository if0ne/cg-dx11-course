#include "SunSystemGame.h"

#include <SimpleMath.h>

#include "../Game.h"
#include "../Camera.h"
#include "../CameraController.h"
#include "../FreeCameraController.h"
#include "../OrbitCamera.h"

#include "PlanetComponent.h"

SunSystemGame::SunSystemGame() : GameComponent(), center_(DirectX::SimpleMath::Vector3::Zero) {
    camera_ = new Camera();
    cameraController_ = new FreeCameraController(*camera_);

    float prevDist = 0.0;
    for (int i = 0; i < 9; i++) {
        float distance = prevDist + (rand() % 10 + 5);
        prevDist += distance;
        planets_.push_back(new PlanetComponent(*this, DirectX::SimpleMath::Vector3(distance, 0.0, 0.0)));
    }
}

SunSystemGame::~SunSystemGame() {
    delete cameraController_;
    delete camera_;

    for (auto planet : planets_) {
        delete planet;
    }
}

void SunSystemGame::Initialize() {
    D3DCompileFromFile(
        L"./shaders/SunSystem.hlsl",
        nullptr,
        nullptr,
        "VSMain",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
        0,
        &vertexBC_,
        nullptr
    );

    D3DCompileFromFile(
        L"./shaders/SunSystem.hlsl",
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
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
        D3D11_INPUT_ELEMENT_DESC {
            "COLOR",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        }
    };

    ctx_.GetRenderContext().GetDevice()->CreateInputLayout(
        inputElements,
        2,
        vertexBC_->GetBufferPointer(),
        vertexBC_->GetBufferSize(),
        &layout_
    );

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
    constBufDesc.ByteWidth = sizeof(DirectX::SimpleMath::Matrix);

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&constBufDesc, nullptr, &wvpBuffer_);

    constBufDesc = {};
    constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
    constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constBufDesc.MiscFlags = 0;
    constBufDesc.StructureByteStride = 0;
    constBufDesc.ByteWidth = sizeof(DirectX::SimpleMath::Matrix);

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&constBufDesc, nullptr, &modelBuffer_);

    for (auto& planet : planets_) {
        planet->Initialize();
    }
}

void SunSystemGame::Update(float deltaTime) {
    cameraController_->Update(deltaTime);

    for (auto& planet : planets_) {
        planet->Update(deltaTime);
    }
}

void SunSystemGame::Draw() {
    ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
    ctx_.GetRenderContext().GetContext()->IASetInputLayout(layout_);
    ctx_.GetRenderContext().GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(0, 1, &wvpBuffer_);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(1, 1, &modelBuffer_);

    auto matrix = camera_->GetCameraMatrix();

    D3D11_MAPPED_SUBRESOURCE res = {};
    ctx_.GetRenderContext().GetContext()->Map(wvpBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &matrix, sizeof(DirectX::SimpleMath::Matrix));
    ctx_.GetRenderContext().GetContext()->Unmap(wvpBuffer_, 0);

    for (auto& planet : planets_) {
        planet->Draw();
    }
}

void SunSystemGame::Reload() {
    for (auto& planet : planets_) {
        planet->Reload();
    }
}

void SunSystemGame::DestroyResources() {
    layout_->Release();
    vertexShader_->Release();
    pixelShader_->Release();
    vertexBC_->Release();
    pixelBC_->Release();
    rastState_->Release();
    wvpBuffer_->Release();
    modelBuffer_->Release();

    for (auto& planet : planets_) {
        planet->DestroyResources();
    }
}

void SunSystemGame::UpdateModelBuffer(DirectX::SimpleMath::Matrix& matrix) {
    D3D11_MAPPED_SUBRESOURCE res = {};
    ctx_.GetRenderContext().GetContext()->Map(modelBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &matrix, sizeof(DirectX::SimpleMath::Matrix));
    ctx_.GetRenderContext().GetContext()->Unmap(modelBuffer_, 0);
}
