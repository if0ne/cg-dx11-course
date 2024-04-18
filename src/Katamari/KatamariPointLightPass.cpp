#include "KatamariPointLightPass.h"

#include "../Game.h"
#include "../Camera.h"
#include "../RenderContext.h"
#include "../Window.h"
#include "../ModelComponent.h"
#include "../QuadComponent.h"
#include "../MeshComponent.h"
#include "../SphereComponent.h"
#include "../AssetLoader.h"

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
#include "../PointLightComponent.h"

using namespace DirectX::SimpleMath;

KatamariPointLightPass::KatamariPointLightPass(
    std::string&& shaderPath, 
    std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr, 
    CD3D11_RASTERIZER_DESC rastState, 
    KatamariGame& game
) :
    game_(game),
    RenderPass(std::move(shaderPath), std::move(vertexAttr), rastState)
{
    sphere_ = new SphereComponent(Vector3::Zero, Vector3::Zero);
}

KatamariPointLightPass::~KatamariPointLightPass() {
    delete sphere_;
}

void KatamariPointLightPass::Initialize() {
    RenderPass::Initialize();

    sphere_->Initialize();

    pointLightBuffer_ = CreateBuffer(sizeof(PointLightData));
    modelBuffer_ = CreateBuffer(sizeof(Matrix));
    viewPosBuffer_ = CreateBuffer(sizeof(Vector4));
    cameraBuffer_ = CreateBuffer(sizeof(Matrix));

    CD3D11_RASTERIZER_DESC pointRastDesc = {};
    pointRastDesc.CullMode = D3D11_CULL_BACK;
    pointRastDesc.FillMode = D3D11_FILL_SOLID;
    ctx_.GetRenderContext().GetDevice()->CreateRasterizerState(&pointRastDesc, &insideState_);

    D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc{};
    depthStencilStateDesc.DepthEnable = true;
    depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    ctx_.GetRenderContext().GetDevice()->CreateDepthStencilState(&depthStencilStateDesc, &depthState_);

    depthStencilStateDesc.DepthEnable = true;
    depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    ctx_.GetRenderContext().GetDevice()->CreateDepthStencilState(&depthStencilStateDesc, &insideDepthState_);
}

void KatamariPointLightPass::Execute() {
    auto gData = game_.mainPass_->geometryPass_->RenderData();

    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(0, 1, &modelBuffer_);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(1, 1, &cameraBuffer_);

    ctx_.GetRenderContext().GetContext()->PSSetConstantBuffers(2, 1, &pointLightBuffer_);
    ctx_.GetRenderContext().GetContext()->PSSetConstantBuffers(3, 1, &viewPosBuffer_);

    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(0, 1, &gData.srvs[0]);
    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(1, 1, &gData.srvs[1]);
    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(2, 1, &gData.srvs[2]);
    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(3, 1, &gData.srvs[3]);

    auto viewPos = game_.camera_->Position();
    UpdateBuffer(viewPosBuffer_, &viewPos, sizeof(Vector3));

    auto camera = game_.camera_->CameraMatrix();
    UpdateBuffer(cameraBuffer_, &camera, sizeof(Matrix));

    // foreach loop
    for (auto& light : game_.pointLights_) {
        auto pointLightData = light->RenderData();
        auto translate = Matrix::CreateTranslation(Vector3(pointLightData.position.x, pointLightData.position.y, pointLightData.position.z));
        auto scale = Matrix::CreateScale(pointLightData.position.w);
        auto matrix = scale * translate;

        UpdateBuffer(modelBuffer_, &matrix, sizeof(Matrix));

        if (light->IsIntersect(game_.camera_->Position())) {
            ctx_.GetRenderContext().GetContext()->RSSetState(insideState_);
            ctx_.GetRenderContext().GetContext()->OMSetDepthStencilState(insideDepthState_, 0);
        } else {
            ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
            ctx_.GetRenderContext().GetContext()->OMSetDepthStencilState(depthState_, 0);
        }

        UpdateBuffer(pointLightBuffer_, &pointLightData, sizeof(PointLightData));

        ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
        ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);

        sphere_->Draw();
    }
}

void KatamariPointLightPass::DestroyResources() {
    RenderPass::DestroyResources();

    sphere_->DestroyResources();
}
