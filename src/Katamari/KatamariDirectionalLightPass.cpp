#include "KatamariDirectionalLightPass.h"

#include "../Game.h"
#include "../Camera.h"
#include "../RenderContext.h"
#include "../Window.h"
#include "../ModelComponent.h"
#include "../QuadComponent.h"
#include "../MeshComponent.h"

#include "KatamariGame.h"
#include "KatamariRenderPass.h"
#include "KatamariCSMPass.h"
#include "KatamariGeometryPass.h"
#include "PlayerComponent.h"
#include "StickyObjectComponent.h"

#include <SimpleMath.h>
#include <array>
#include "../CascadedShadowMap.h"
#include "../AmbientLightComponent.h"
#include "../DirectionalLightComponent.h"

using namespace DirectX::SimpleMath;

KatamariDirectionalLightPass::KatamariDirectionalLightPass(
    std::string&& shaderPath, 
    std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr, 
    CD3D11_RASTERIZER_DESC rastState, 
    KatamariGame& game
) :
    game_(game),
    RenderPass(std::move(shaderPath), std::move(vertexAttr), rastState)
{
    quad_ = new QuadComponent();
}

KatamariDirectionalLightPass::~KatamariDirectionalLightPass() {
    delete quad_;
}

void KatamariDirectionalLightPass::Initialize() {
    RenderPass::Initialize();

    quad_->Initialize();

    dirLightBuffer_ = CreateBuffer(sizeof(DirectionalLightData));
    ambientLightBuffer_ = CreateBuffer(sizeof(AmbientLightData));
    cascadeBuffer_ = CreateBuffer(sizeof(CascadedShadowMapData));
    screenSizeBuffer_ = CreateBuffer(sizeof(Vector4));
    viewPosBuffer_ = CreateBuffer(sizeof(Vector4));
}

void KatamariDirectionalLightPass::Execute() {
    auto csmData = game_.mainPass_->csmPass_->RenderData();
    auto gData = game_.mainPass_->geometryPass_->RenderData();

    ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
    ctx_.GetRenderContext().GetContext()->IASetInputLayout(layout_);
    ctx_.GetRenderContext().GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);

    ctx_.GetRenderContext().GetContext()->PSSetConstantBuffers(0, 1, &dirLightBuffer_);
    ctx_.GetRenderContext().GetContext()->PSSetConstantBuffers(1, 1, &ambientLightBuffer_);
    ctx_.GetRenderContext().GetContext()->PSSetConstantBuffers(2, 1, &cascadeBuffer_);
    ctx_.GetRenderContext().GetContext()->PSSetConstantBuffers(3, 1, &screenSizeBuffer_);
    ctx_.GetRenderContext().GetContext()->PSSetConstantBuffers(4, 1, &viewPosBuffer_);

    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(0, 1, &gData.srvs[0]);
    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(1, 1, &gData.srvs[1]);
    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(2, 1, &gData.srvs[2]);
    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(3, 1, &gData.srvs[3]);
    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(5, 1, &csmData.srv);
    ctx_.GetRenderContext().GetContext()->PSSetSamplers(0, 1, &csmData.sampler);

    UpdateBuffer(cascadeBuffer_, &csmData.cascades, sizeof(CascadedShadowMapData));

    auto dirLightData = game_.directionalLight_->RenderData();

    UpdateBuffer(dirLightBuffer_, &dirLightData, sizeof(DirectionalLightData));

    auto ambientData = game_.ambientLight_->RenderData();
    UpdateBuffer(ambientLightBuffer_, &ambientData, sizeof(AmbientLightData));

    Vector4 screenSize = Vector4(ctx_.GetWindow().GetWidth(), ctx_.GetWindow().GetHeight(), 0.0, 0.0);
    UpdateBuffer(screenSizeBuffer_, &screenSize, sizeof(Vector4));

    auto viewPos = game_.camera_->Position();
    UpdateBuffer(viewPosBuffer_, &viewPos, sizeof(Vector3));

    quad_->Draw();
}

void KatamariDirectionalLightPass::DestroyResources() {
    RenderPass::DestroyResources();

    quad_->DestroyResources();
}
