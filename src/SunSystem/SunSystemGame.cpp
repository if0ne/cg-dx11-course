#include "SunSystemGame.h"

#include <SimpleMath.h>

#include "../Game.h"
#include "../InputDevice.h"
#include "../Camera.h"
#include "../CameraController.h"
#include "../FreeCameraController.h"
#include "../OrbitCameraController.h"
#include "../SphereComponent.h"

#include "PlanetComponent.h"

#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

SunSystemGame::SunSystemGame() : GameComponent(), center_(DirectX::SimpleMath::Vector3::Zero) {
    camera_ = new Camera();
    freeCameraController_ = new FreeCameraController(*camera_);
    orbitCameraController_ = new OrbitCameraController(*camera_, Vector3(0.0, 0.0, 0.1), &center_);
    currentCameraController_ = freeCameraController_;

    cameraT_ = new Camera();
    top_ = new OrbitCameraController(*cameraT_, Vector3(0.0, 250.0, 10.0), &center_);
    top_->Active(false);

    cameraR_ = new Camera();
    right_ = new OrbitCameraController(*cameraR_, Vector3(250.0, 0.0, 0.0), &center_);
    right_->Active(false);

    cameraF_ = new Camera();
    forward_ = new OrbitCameraController(*cameraF_, Vector3(0.0, 0.0, 250.0), &center_);
    forward_->Active(false);

    sun_ = new SphereComponent(Vector3(1.0, 0.8, 0.0), Vector3(1.0, 0.5, 0.0));
    sunSize_ = 109.0;
    
    float ed = 20.0f;

    planets_.push_back(new PlanetComponent(*this, 109.0 + 0.38 * ed, 0.38, 0, Vector3(0.8, 0.8, 0.8), Vector3(0.6, 0.6, 0.6)));
    planets_.push_back(new PlanetComponent(*this, 109.0 + 0.72 * ed, 0.95, 0, Vector3(0.95, 0.7, 0.4), Vector3(0.8, 0.4, 0.1)));
    planets_.push_back(new PlanetComponent(*this, 109.0 + 1.0 * ed, 1.0, 1, Vector3(0.0, 0.5, 1.0), Vector3(0.0, 0.2, 0.6)));
    planets_.push_back(new PlanetComponent(*this, 109.0 + 1.52 * ed, 0.53, 2, Vector3(1.0, 0.4, 0.2), Vector3(0.8, 0.2, 0.0)));
    planets_.push_back(new PlanetComponent(*this, 109.0 + 5.20 * ed, 11.2, 12, Vector3(0.9, 0.7, 0.4), Vector3(0.8, 0.6, 0.2)));
    planets_.push_back(new PlanetComponent(*this, 109.0 + 9.58 * ed, 9.45, 256, Vector3(0.9, 0.9, 0.7), Vector3(0.8, 0.8, 0.5)));
    planets_.push_back(new PlanetComponent(*this, 109.0 + 19.22 * ed, 4.0, 5, Vector3(0.7, 0.9, 0.95), Vector3(0.5, 0.8, 0.9)));
    planets_.push_back(new PlanetComponent(*this, 109.0 + 30.05 * ed, 3.88, 2, Vector3(0.2, 0.4, 0.8), Vector3(0.1, 0.2, 0.6)));
}

SunSystemGame::~SunSystemGame() {
    delete currentCameraController_;
    delete camera_;
    delete sun_;

    for (auto& planet : planets_) {
        delete planet;
    }
}

void SunSystemGame::Initialize() {
    ctx_.GetWindow().HideWindowCursor();

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

    sun_->Initialize();

    for (auto& planet : planets_) {
        planet->Initialize();
    }
    currentPlanet_ = 0;
}

void SunSystemGame::Update(float deltaTime) {
    if (ctx_.GetInputDevice().IsKeyDown(Keys::D1)) {
        currentCameraController_->Active(false);
        currentCameraController_ = freeCameraController_;
        currentCameraController_->Active(true);
    }

    if (ctx_.GetInputDevice().IsKeyDown(Keys::D2)) {
        currentCameraController_->Active(false);
        currentCameraController_ = orbitCameraController_;
        currentCameraController_->Active(true);
    }

    if (ctx_.GetInputDevice().IsKeyDown(Keys::D3)) {
        camera_->UpdatePerspectiveProjection();
    }

    if (ctx_.GetInputDevice().IsKeyDown(Keys::D4)) {
        camera_->UpdateOrthoProjection();
    }

    if (ctx_.GetInputDevice().IsKeyDown(Keys::D5)) {
        auto controller = dynamic_cast<OrbitCameraController*>(currentCameraController_);

        if (controller) {
            controller->Target(&center_);
        }
    }

    if (ctx_.GetInputDevice().IsKeyDown(Keys::D6)) {
        auto controller = dynamic_cast<OrbitCameraController*>(currentCameraController_);

        if (controller) {
            controller->Target(planets_[currentPlanet_]->GetRefGlobalPosition());

            currentPlanet_ = (currentPlanet_ + 1) % planets_.size();
        }
    }

    if (ctx_.GetInputDevice().IsKeyDown(Keys::LeftShift)) {
        ctx_.GetWindow().ShowWindowCursor();
    } else {
        ctx_.GetWindow().HideWindowCursor();
        ctx_.GetWindow().LockWindowCursor();
    }

    currentCameraController_->Update(deltaTime);
    top_->Update(deltaTime);
    right_->Update(deltaTime);
    forward_->Update(deltaTime);

    sun_->Update(deltaTime);
    for (auto& planet : planets_) {
        planet->Update(deltaTime);
    }
}

void SunSystemGame::Draw() {
    ctx_.SetViewport(0, 0, 620, 360);
    ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
    ctx_.GetRenderContext().GetContext()->IASetInputLayout(layout_);
    ctx_.GetRenderContext().GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(0, 1, &wvpBuffer_);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(1, 1, &modelBuffer_);

    auto matrix = camera_->CameraMatrix();

    D3D11_MAPPED_SUBRESOURCE res = {};
    ctx_.GetRenderContext().GetContext()->Map(wvpBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &matrix, sizeof(DirectX::SimpleMath::Matrix));
    ctx_.GetRenderContext().GetContext()->Unmap(wvpBuffer_, 0);

    auto sunMatrix = DirectX::SimpleMath::Matrix::CreateScale(sunSize_);
    UpdateModelBuffer(sunMatrix);
    sun_->Draw();

    for (auto& planet : planets_) {
        planet->Draw();
    }

    ctx_.SetViewport(620, 0, 620, 360);
    ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
    ctx_.GetRenderContext().GetContext()->IASetInputLayout(layout_);
    ctx_.GetRenderContext().GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(0, 1, &wvpBuffer_);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(1, 1, &modelBuffer_);

    matrix = cameraT_->CameraMatrix();

    res = {};
    ctx_.GetRenderContext().GetContext()->Map(wvpBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &matrix, sizeof(DirectX::SimpleMath::Matrix));
    ctx_.GetRenderContext().GetContext()->Unmap(wvpBuffer_, 0);

    sunMatrix = DirectX::SimpleMath::Matrix::CreateScale(sunSize_);
    UpdateModelBuffer(sunMatrix);
    sun_->Draw();

    for (auto& planet : planets_) {
        planet->Draw();
    }

    ctx_.SetViewport(0, 320, 620, 360);
    ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
    ctx_.GetRenderContext().GetContext()->IASetInputLayout(layout_);
    ctx_.GetRenderContext().GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(0, 1, &wvpBuffer_);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(1, 1, &modelBuffer_);

    matrix = cameraF_->CameraMatrix();

    res = {};
    ctx_.GetRenderContext().GetContext()->Map(wvpBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &matrix, sizeof(DirectX::SimpleMath::Matrix));
    ctx_.GetRenderContext().GetContext()->Unmap(wvpBuffer_, 0);

    sunMatrix = DirectX::SimpleMath::Matrix::CreateScale(sunSize_);
    UpdateModelBuffer(sunMatrix);
    sun_->Draw();

    for (auto& planet : planets_) {
        planet->Draw();
    }

    ctx_.SetViewport(620, 320, 620, 360);
    ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
    ctx_.GetRenderContext().GetContext()->IASetInputLayout(layout_);
    ctx_.GetRenderContext().GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(0, 1, &wvpBuffer_);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(1, 1, &modelBuffer_);

    matrix = cameraR_->CameraMatrix();

    res = {};
    ctx_.GetRenderContext().GetContext()->Map(wvpBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &matrix, sizeof(DirectX::SimpleMath::Matrix));
    ctx_.GetRenderContext().GetContext()->Unmap(wvpBuffer_, 0);

    sunMatrix = DirectX::SimpleMath::Matrix::CreateScale(sunSize_);
    UpdateModelBuffer(sunMatrix);
    sun_->Draw();

    for (auto& planet : planets_) {
        planet->Draw();
    }
}

void SunSystemGame::Reload() {
    sun_->Reload();
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

    sun_->DestroyResources();
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
