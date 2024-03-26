#include "KatamariGame.h"

#include <string>
#include <vector>
#include <SimpleMath.h>

#include "../AssetLoader.h"
#include "../Game.h"
#include "../InputDevice.h"
#include "../Camera.h"
#include "../OrbitCameraController.h"
#include "../SphereComponent.h"

#include "PlayerComponent.h"
#include "StickyObjectComponent.h"

using namespace DirectX::SimpleMath;

KatamariGame::KatamariGame() : GameComponent() {
    camera_ = new Camera();
    player_ = new PlayerComponent(*camera_, *this);

    auto path = std::string("/bball.fbx");
    std::vector<std::pair<std::string, float>> models;
    models.push_back(std::make_pair(std::string("/rock.fbx"), 1.0));
    models.push_back(std::make_pair(std::string("/pump.fbx"), 1.0));
    models.push_back(std::make_pair(std::string("/wheel.fbx"), 0.005));

    for (int i = 0; i < 10; i++) {
        int random = rand() % models.size();
        int x = rand() % 40 - 20;
        int z = rand() % 40 - 20;
        objects_.push_back(new StickyObjectComponent(models[random].first, models[random].second, Vector3(x, 0, z), *this));
    }
}

KatamariGame::~KatamariGame() {
    delete player_;
    delete camera_;

    for (auto& object : objects_) {
        delete object;
    }
}

void KatamariGame::Initialize() {
    ctx_.GetWindow().HideWindowCursor();

    D3DCompileFromFile(
        L"./shaders/Katamari.hlsl",
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
        L"./shaders/Katamari.hlsl",
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
            "NORMAL",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
        D3D11_INPUT_ELEMENT_DESC {
            "TEXCOORD",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0,
            D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        }
    };

    ctx_.GetRenderContext().GetDevice()->CreateInputLayout(
        inputElements,
        3,
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

    player_->Initialize();

    for (auto& object : objects_) {
        object->Initialize();
    }

    auto path = std::string("/Floor_plate.fbx");
    ground_ = ctx_.GetAssetLoader().LoadModel(path);
}

void KatamariGame::Update(float deltaTime) {
    if (ctx_.GetInputDevice().IsKeyDown(Keys::LeftShift)) {
        ctx_.GetWindow().ShowWindowCursor();
    } else {
        ctx_.GetWindow().HideWindowCursor();
        ctx_.GetWindow().LockWindowCursor();
    }

    player_->Update(deltaTime);

    for (auto& object : objects_) {
        object->Update(deltaTime);
    }
}

void KatamariGame::Draw() {
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

    player_->Draw();

    for (auto& object : objects_) {
        object->Draw();
    }

    matrix = Matrix::CreateTranslation(Vector3(0, -4.0, 0));
    UpdateModelBuffer(matrix);
    ground_->Draw();
}

void KatamariGame::Reload() {
    player_->Reload();
    for (auto& object : objects_) {
        object->Reload();
    }
}

void KatamariGame::DestroyResources() {
    layout_->Release();
    vertexShader_->Release();
    pixelShader_->Release();
    vertexBC_->Release();
    pixelBC_->Release();
    rastState_->Release();
    wvpBuffer_->Release();
    modelBuffer_->Release();

    player_->DestroyResources();
    for (auto& object : objects_) {
        object->DestroyResources();
    }
}

void KatamariGame::UpdateModelBuffer(DirectX::SimpleMath::Matrix& matrix) {
    D3D11_MAPPED_SUBRESOURCE res = {};
    ctx_.GetRenderContext().GetContext()->Map(modelBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &matrix, sizeof(DirectX::SimpleMath::Matrix));
    ctx_.GetRenderContext().GetContext()->Unmap(modelBuffer_, 0);
}
