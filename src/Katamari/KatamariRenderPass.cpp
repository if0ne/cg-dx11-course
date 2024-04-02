#include "KatamariRenderPass.h"

#include "../Game.h"
#include "../RenderContext.h"
#include "../Window.h"
#include "../Camera.h"
#include "../DirectionalLightComponent.h"
#include "../AmbientLightComponent.h"
#include "../PointLightComponent.h"
#include "../ModelComponent.h"
#include "../MeshComponent.h"
#include "../CascadedShadowMap.h"

#include "SimpleMath.h"

#include "KatamariGame.h"
#include "PlayerComponent.h"
#include "StickyObjectComponent.h"
#include "KatamariCSMPass.h"

using namespace DirectX::SimpleMath;


KatamariRenderPass::KatamariRenderPass(
    std::string&& shaderPath,
    std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr,
    KatamariGame& game
) :
    game_(game),
    RenderPass(std::move(shaderPath), std::move(vertexAttr))
{
    std::string path = "./shaders/CascadeShader.hlsl";
    std::vector<std::pair<const char*, DXGI_FORMAT>> csmVertexAttr{
        std::make_pair("POSITION", DXGI_FORMAT_R32G32B32_FLOAT),
        std::make_pair("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT),
        std::make_pair("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT),
        std::make_pair("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
    };

    csmPass_ = new KatamariCSMPass(std::move(path), std::move(csmVertexAttr), game_);
}

KatamariRenderPass::~KatamariRenderPass() {
    delete csmPass_;
}

void KatamariRenderPass::Initialize() {
    RenderPass::Initialize();

    wvpBuffer_ = CreateBuffer(sizeof(DirectX::SimpleMath::Matrix));
    modelBuffer_ = CreateBuffer(sizeof(DirectX::SimpleMath::Matrix));
    dirLightBuffer_ = CreateBuffer(sizeof(DirectionalLightData));
    pointLightBuffer_ = CreateBuffer(sizeof(PointLightData));
    ambientLightBuffer_ = CreateBuffer(sizeof(AmbientLightData));
    materialBuffer_ = CreateBuffer(sizeof(Material));
    viewPosBuffer_ = CreateBuffer(sizeof(Vector4));
    cascadeBuffer_ = CreateBuffer(sizeof(CascadedShadowMapData));

    csmPass_->Initialize();
}

void KatamariRenderPass::Execute() {
    csmPass_->Execute();

    auto csmData = csmPass_->RenderData();

    auto rt = ctx_.GetWindow().GetRenderTarget();
    auto ds = ctx_.GetWindow().GetDepthStencilView();

    ctx_.GetRenderContext().GetContext()->OMSetRenderTargets(1, &rt, ds);

    float color[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    ctx_.GetRenderContext().GetContext()->ClearRenderTargetView(rt, color);
    ctx_.GetRenderContext().GetContext()->ClearDepthStencilView(ds, D3D11_CLEAR_DEPTH, 1.0, 0);

    ctx_.GetRenderContext().ActivateDepthStencilState();

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
    ctx_.GetRenderContext().GetContext()->PSSetConstantBuffers(9, 1, &cascadeBuffer_);

    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(2, 1, &csmData.srv);
    ctx_.GetRenderContext().GetContext()->PSSetSamplers(1, 1, &csmData.sampler);

    UpdateBuffer(cascadeBuffer_, &csmData.cascades, sizeof(CascadedShadowMapData));

    auto dirLightData = game_.directionalLight_->RenderData();

    UpdateBuffer(dirLightBuffer_, &dirLightData, sizeof(DirectionalLightData));

    auto ambientData = game_.ambientLight_->RenderData();
    UpdateBuffer(ambientLightBuffer_, &ambientData, sizeof(AmbientLightData));

    auto pointData = game_.pointLight_->RenderData();
    UpdateBuffer(pointLightBuffer_, &pointData, sizeof(PointLightData));

    auto matrix = game_.camera_->CameraMatrix();
    UpdateBuffer(wvpBuffer_, &matrix, sizeof(Matrix));

    auto viewPos = game_.camera_->Position();
    UpdateBuffer(viewPosBuffer_, &viewPos, sizeof(Vector4));

    auto renderData = game_.player_->GetRenderData();

    UINT strides[] = { sizeof(Vertex) };
    UINT offsets[] = { 0 };

    UpdateBuffer(modelBuffer_, &renderData.transform, sizeof(Matrix));
    UpdateBuffer(materialBuffer_, &renderData.material, sizeof(Material));

    for (auto& mesh : renderData.meshData) {
        ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(mesh.ib, DXGI_FORMAT_R32_UINT, 0);
        ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &mesh.vb, strides, offsets);
        ctx_.GetRenderContext().GetContext()->PSSetShaderResources(0, 1, &mesh.texture);
        ctx_.GetRenderContext().GetContext()->PSSetShaderResources(1, 1, &mesh.normal);
        ctx_.GetRenderContext().GetContext()->PSSetSamplers(0, 1, &mesh.sampler);

        ctx_.GetRenderContext().GetContext()->DrawIndexed(mesh.indexCount, 0, 0);
    }


    for (auto& object : game_.objects_) {
        renderData = object->GetRenderData();
        UpdateBuffer(modelBuffer_, &renderData.transform, sizeof(Matrix));
        UpdateBuffer(materialBuffer_, &renderData.material, sizeof(Material));

        for (auto& mesh : renderData.meshData) {
            ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(mesh.ib, DXGI_FORMAT_R32_UINT, 0);
            ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &mesh.vb, strides, offsets);
            ctx_.GetRenderContext().GetContext()->PSSetShaderResources(0, 1, &mesh.texture);
            ctx_.GetRenderContext().GetContext()->PSSetShaderResources(1, 1, &mesh.normal);
            ctx_.GetRenderContext().GetContext()->PSSetSamplers(0, 1, &mesh.sampler);

            ctx_.GetRenderContext().GetContext()->DrawIndexed(mesh.indexCount, 0, 0);
        }
    }
    
    auto groundMatrix = Matrix::CreateTranslation(Vector3(0, -4.0, 0));
    UpdateBuffer(modelBuffer_, &groundMatrix, sizeof(Matrix));

    auto mat = Material{
        Vector4(1.0, 1.0, 1.0, 1.0),
        1.0,
        0.01,
        1.0,
        0.0
    };
    UpdateBuffer(materialBuffer_, &mat, sizeof(Material));
    auto mesh = game_.ground_->GetMeshRenderData();
    
    for (auto& mesh : mesh) {
        ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(mesh.ib, DXGI_FORMAT_R32_UINT, 0);
        ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &mesh.vb, strides, offsets);
        ctx_.GetRenderContext().GetContext()->PSSetShaderResources(0, 1, &mesh.texture);
        ctx_.GetRenderContext().GetContext()->PSSetShaderResources(1, 1, &mesh.normal);
        ctx_.GetRenderContext().GetContext()->PSSetSamplers(0, 1, &mesh.sampler);

        ctx_.GetRenderContext().GetContext()->DrawIndexed(mesh.indexCount, 0, 0);
    }
}

void KatamariRenderPass::DestroyResources() {
    csmPass_->DestroyResources();
}
