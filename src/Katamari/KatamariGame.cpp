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
#include "../DirectionalLightComponent.h"
#include "../AmbientLightComponent.h"
#include "../PointLightComponent.h"
#include "../ModelComponent.h"

#include "PlayerComponent.h"
#include "StickyObjectComponent.h"

using namespace DirectX::SimpleMath;

KatamariGame::KatamariGame() : GameComponent() {
    camera_ = new Camera();
    player_ = new PlayerComponent(*camera_, *this);

    auto path = std::string("/bball.fbx");
    std::vector<std::tuple<std::string, float, Material>> models;
    models.push_back(std::make_tuple(std::string("/rock.fbx"), 1.0, Material{
        Vector4(1.0, 1.0, 1.0, 1.0),
        1.0,
        1.0,
        1.0,
        0.0
    }));

    models.push_back(std::make_tuple(std::string("/pump.fbx"), 1.0, Material{
        Vector4(1.0, 1.0, 1.0, 1.0),
        1.0,
        1.0,
        1.0,
        0.0,
    }));

    models.push_back(std::make_tuple(std::string("/wheel.fbx"), 0.005, Material{
        Vector4(1.0, 1.0, 1.0, 1.0),
        1.0,
        1.0,
        1.0,
        0.0
    }));

    for (int i = 0; i < 10; i++) {
        int random = rand() % models.size();
        int x = rand() % 40 - 20;
        int z = rand() % 40 - 20;
        objects_.push_back(new StickyObjectComponent(
            std::get<0>(models[random]), 
            std::get<1>(models[random]), 
            Vector3(x, 0, z), 
            std::get<2>(models[random]), 
            *this
        ));
    }

    directionalLight_ = new DirectionalLightComponent(Vector3(0.25, -1.0, 0.0), Vector3(1.0, 1.0, 1.0), 1.0);
    ambientLight_ = new AmbientLightComponent(Vector3(0.04, 0.14, 0.72), 0.23);
    pointLight_ = new PointLightComponent(Vector3(0.0, 4.0, 0.0), 16.0, Vector3(0.94, 0.14, 0.0), 5.0);
}

KatamariGame::~KatamariGame() {
    delete player_;
    delete camera_;
    delete directionalLight_;
    delete ambientLight_;

    for (auto& object : objects_) {
        delete object;
    }
}

void KatamariGame::Initialize() {
    ctx_.GetWindow().HideWindowCursor();

    D3DCompileFromFile(
        L"./shaders/KatamariLight.hlsl",
        nullptr,
        nullptr,
        "VSMain",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
        0,
        &vertexBC_,
        nullptr
    );
;
    D3DCompileFromFile(
        L"./shaders/KatamariLight.hlsl",
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

    constBufDesc = {};
    constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
    constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constBufDesc.MiscFlags = 0;
    constBufDesc.StructureByteStride = 0;
    constBufDesc.ByteWidth = sizeof(DirectionalLightData);

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&constBufDesc, nullptr, &dirLightBuffer_);

    constBufDesc = {};
    constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
    constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constBufDesc.MiscFlags = 0;
    constBufDesc.StructureByteStride = 0;
    constBufDesc.ByteWidth = sizeof(PointLightData);

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&constBufDesc, nullptr, &pointLightBuffer_);

    constBufDesc = {};
    constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
    constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constBufDesc.MiscFlags = 0;
    constBufDesc.StructureByteStride = 0;
    constBufDesc.ByteWidth = sizeof(AmbientLightData);

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&constBufDesc, nullptr, &ambientLightBuffer_);

    constBufDesc = {};
    constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
    constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constBufDesc.MiscFlags = 0;
    constBufDesc.StructureByteStride = 0;
    constBufDesc.ByteWidth = sizeof(Material);

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&constBufDesc, nullptr, &materialBuffer_);

    constBufDesc = {};
    constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
    constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constBufDesc.MiscFlags = 0;
    constBufDesc.StructureByteStride = 0;
    constBufDesc.ByteWidth = sizeof(Vector4);

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&constBufDesc, nullptr, &viewPosBuffer_);

    player_->Initialize();

    for (auto& object : objects_) {
        object->Initialize();
    }

    auto path = std::string("/Floor_plate.fbx");
    ground_ = ctx_.GetAssetLoader().LoadModel(path);
}

void KatamariGame::Update(float deltaTime) {
    if (ctx_.GetInputDevice().IsKeyDown(Keys::Escape)) {
        ctx_.Exit();
    }

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

    ctx_.GetRenderContext().GetContext()->PSSetConstantBuffers(2, 1, &dirLightBuffer_);
    ctx_.GetRenderContext().GetContext()->PSSetConstantBuffers(3, 1, &ambientLightBuffer_);
    ctx_.GetRenderContext().GetContext()->PSSetConstantBuffers(4, 1, &pointLightBuffer_);
    ctx_.GetRenderContext().GetContext()->PSSetConstantBuffers(7, 1, &viewPosBuffer_);
    ctx_.GetRenderContext().GetContext()->PSSetConstantBuffers(8, 1, &materialBuffer_);

    auto dirLightData = directionalLight_->RenderData();

    D3D11_MAPPED_SUBRESOURCE res = {};
    ctx_.GetRenderContext().GetContext()->Map(dirLightBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &dirLightData, sizeof(DirectionalLightData));
    ctx_.GetRenderContext().GetContext()->Unmap(dirLightBuffer_, 0);

    auto ambientData = ambientLight_->RenderData();
    ctx_.GetRenderContext().GetContext()->Map(ambientLightBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &ambientData, sizeof(AmbientLightData));
    ctx_.GetRenderContext().GetContext()->Unmap(ambientLightBuffer_, 0);

    auto pointData = pointLight_->RenderData();
    ctx_.GetRenderContext().GetContext()->Map(pointLightBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &pointData, sizeof(PointLightData));
    ctx_.GetRenderContext().GetContext()->Unmap(pointLightBuffer_, 0);

    auto matrix = camera_->CameraMatrix();

    res = {};
    ctx_.GetRenderContext().GetContext()->Map(wvpBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &matrix, sizeof(Matrix));
    ctx_.GetRenderContext().GetContext()->Unmap(wvpBuffer_, 0);

    auto viewPos = camera_->Position();

    res = {};
    ctx_.GetRenderContext().GetContext()->Map(viewPosBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &viewPos, sizeof(Vector4));
    ctx_.GetRenderContext().GetContext()->Unmap(viewPosBuffer_, 0);

    player_->Draw();

    for (auto& object : objects_) {
        object->Draw();
    }

    auto groundMatrix = Matrix::CreateTranslation(Vector3(0, -4.0, 0));
    UpdateModelBuffer(groundMatrix);
    UpdateMaterialBuffer(Material{
        Vector4(1.0, 1.0, 1.0, 1.0),
        1.0,
        0.01,
        1.0,
        0.0
    });
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
    dirLightBuffer_->Release();
    ambientLightBuffer_->Release();
    pointLightBuffer_->Release();

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

void KatamariGame::UpdateMaterialBuffer(Material mat) {
    D3D11_MAPPED_SUBRESOURCE res = {};
    ctx_.GetRenderContext().GetContext()->Map(materialBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &mat, sizeof(Material));
    ctx_.GetRenderContext().GetContext()->Unmap(materialBuffer_, 0);
}