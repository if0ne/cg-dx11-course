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
#include "SimpleMath.h"

#include "KatamariGame.h"
#include "PlayerComponent.h"
#include "StickyObjectComponent.h"

using namespace DirectX::SimpleMath;


KatamariRenderPass::KatamariRenderPass(
    std::string&& shaderPath,
    std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr,
    KatamariGame& game
) :
    game_(game),
    RenderPass(std::move(shaderPath), std::move(vertexAttr))
{
}

void KatamariRenderPass::Execute() {
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

    auto dirLightData = game_.directionalLight_->RenderData();

    UpdateBuffer(game_.dirLightBuffer_, &dirLightData, sizeof(DirectionalLightData));

    auto ambientData = game_.ambientLight_->RenderData();
    UpdateBuffer(game_.ambientLightBuffer_, &ambientData, sizeof(AmbientLightData));

    auto pointData = game_.pointLight_->RenderData();
    UpdateBuffer(game_.pointLightBuffer_, &pointData, sizeof(PointLightData));

    auto matrix = game_.camera_->CameraMatrix();
    UpdateBuffer(game_.wvpBuffer_, &matrix, sizeof(Matrix));

    auto viewPos = game_.camera_->Position();
    UpdateBuffer(game_.viewPosBuffer_, &viewPos, sizeof(Vector4));

    auto renderData = game_.player_->GetRenderData();

    UINT strides[] = { sizeof(Vertex) };
    UINT offsets[] = { 0 };

    UpdateBuffer(game_.modelBuffer_, &renderData.transform, sizeof(Matrix));
    UpdateBuffer(game_.materialBuffer_, &renderData.material, sizeof(Material));

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
        UpdateBuffer(game_.modelBuffer_, &renderData.transform, sizeof(Matrix));
        UpdateBuffer(game_.materialBuffer_, &renderData.material, sizeof(Material));

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
    UpdateBuffer(game_.modelBuffer_, &groundMatrix, sizeof(Matrix));

    auto mat = Material{
        Vector4(1.0, 1.0, 1.0, 1.0),
        1.0,
        0.01,
        1.0,
        0.0
    };
    UpdateBuffer(game_.materialBuffer_, &mat, sizeof(Material));
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
