#include "SunSystemGame.h"

#include "../Game.h"
#include "../Camera.h"
#include "../CameraController.h"
#include "../FreeCameraController.h"
#include "../OrbitCamera.h"
#include "../CubeComponent.h"

SunSystemGame::SunSystemGame() : GameComponent(), center_(DirectX::SimpleMath::Vector3::Zero) {
    camera_ = new Camera();
    cameraController_ = new FreeCameraController(*camera_);

    cube_ = new CubeComponent();
}

SunSystemGame::~SunSystemGame() {
    delete cameraController_;
    delete camera_;
    delete cube_;
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

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&constBufDesc, nullptr, &constBuffer_);

    cube_->Initialize();
}

void SunSystemGame::Update(float deltaTime) {
    cameraController_->Update(deltaTime);

    auto matrix = DirectX::SimpleMath::Matrix::Identity * camera_->GetCameraMatrix();

    D3D11_MAPPED_SUBRESOURCE res = {};
    ctx_.GetRenderContext().GetContext()->Map(constBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &matrix, sizeof(DirectX::SimpleMath::Matrix));
    ctx_.GetRenderContext().GetContext()->Unmap(constBuffer_, 0);
}

void SunSystemGame::Draw() {
    ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
    ctx_.GetRenderContext().GetContext()->IASetInputLayout(layout_);
    ctx_.GetRenderContext().GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(0, 1, &constBuffer_);

    cube_->Draw();
}

void SunSystemGame::Reload() {
}

void SunSystemGame::DestroyResources() {
    layout_->Release();
    vertexShader_->Release();
    pixelShader_->Release();
    vertexBC_->Release();
    pixelBC_->Release();
    rastState_->Release();
    constBuffer_->Release();

    cube_->DestroyResources();
}
