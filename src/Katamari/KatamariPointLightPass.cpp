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

    CD3D11_RASTERIZER_DESC unmarkPass
    {
        D3D11_FILL_SOLID,
        D3D11_CULL_NONE,
        TRUE,
        D3D11_DEFAULT_DEPTH_BIAS,
        D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
        D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
        TRUE,
        FALSE,
        FALSE,
        FALSE
    };

    ctx_.GetRenderContext().GetDevice()->CreateRasterizerState(&unmarkPass, &unmarkState_);

    D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc{};

    depthStencilStateDesc = {};

    depthStencilStateDesc.DepthEnable = TRUE;
    depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_GREATER;
    depthStencilStateDesc.StencilEnable = TRUE;
    depthStencilStateDesc.StencilReadMask = 0xff;
    depthStencilStateDesc.StencilWriteMask = 0xff;
    depthStencilStateDesc.FrontFace = D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP ,D3D11_STENCIL_OP_KEEP ,D3D11_STENCIL_OP_DECR_SAT ,D3D11_COMPARISON_ALWAYS };
    depthStencilStateDesc.BackFace = D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP ,D3D11_STENCIL_OP_KEEP ,D3D11_STENCIL_OP_DECR_SAT ,D3D11_COMPARISON_ALWAYS };

    ctx_.GetRenderContext().GetDevice()->CreateDepthStencilState(&depthStencilStateDesc, &unmarkStencilState_);

    depthStencilStateDesc = {};

    depthStencilStateDesc.DepthEnable = TRUE;
    depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
    depthStencilStateDesc.StencilEnable = TRUE;
    depthStencilStateDesc.StencilReadMask = 0xff;
    depthStencilStateDesc.StencilWriteMask = 0xff;
    depthStencilStateDesc.FrontFace = D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP ,D3D11_STENCIL_OP_KEEP ,D3D11_STENCIL_OP_KEEP , D3D11_COMPARISON_EQUAL };
    depthStencilStateDesc.BackFace = D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP ,D3D11_STENCIL_OP_KEEP ,D3D11_STENCIL_OP_KEEP ,D3D11_COMPARISON_EQUAL };

    ctx_.GetRenderContext().GetDevice()->CreateDepthStencilState(&depthStencilStateDesc, &litStencilState_);
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

        UpdateBuffer(pointLightBuffer_, &pointLightData, sizeof(PointLightData));

        ctx_.GetRenderContext().GetContext()->IASetInputLayout(layout_);
        ctx_.GetRenderContext().GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
        ctx_.GetRenderContext().GetContext()->PSSetShader(nullptr, nullptr, 0);

        ctx_.GetRenderContext().GetContext()->RSSetState(unmarkState_);
        ctx_.GetRenderContext().GetContext()->OMSetDepthStencilState(unmarkStencilState_, 1);

        sphere_->Draw();

        ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
        ctx_.GetRenderContext().GetContext()->OMSetDepthStencilState(litStencilState_, 1);
        ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
        ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);

        sphere_->Draw();
    }
}

void KatamariPointLightPass::DestroyResources() {
    RenderPass::DestroyResources();

    sphere_->DestroyResources();
}
