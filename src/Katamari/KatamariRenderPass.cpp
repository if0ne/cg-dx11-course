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
#include "../QuadComponent.h"
#include "../CascadedShadowMap.h"
#include "../ParticleSystemComponent.h"

#include "SimpleMath.h"

#include "KatamariGame.h"
#include "PlayerComponent.h"
#include "StickyObjectComponent.h"
#include "KatamariCSMPass.h"
#include "KatamariShadowMapPass.h"
#include "KatamariGeometryPass.h"
#include "KatamariDirectionalLightPass.h"
#include "KatamariPointLightPass.h"

#include <iostream>

using namespace DirectX::SimpleMath;


KatamariRenderPass::KatamariRenderPass(
    std::string&& shaderPath,
    std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr,
    CD3D11_RASTERIZER_DESC rastState,
    KatamariGame& game
) :
    game_(game),
    RenderPass(std::move(shaderPath), std::move(vertexAttr), rastState)
{
    std::string path = "./shaders/CascadeShader.hlsl";
    std::vector<std::pair<const char*, DXGI_FORMAT>> csmVertexAttr{
        std::make_pair("POSITION", DXGI_FORMAT_R32G32B32_FLOAT),
        std::make_pair("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT),
        std::make_pair("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT),
        std::make_pair("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
    };

    CD3D11_RASTERIZER_DESC shadowMapRast
    {
        D3D11_FILL_SOLID,
        D3D11_CULL_NONE,
        FALSE,
        10000,
        0.0,
        1.0,
        TRUE,
        FALSE,
        FALSE,
        FALSE
    };
    csmPass_ = new KatamariCSMPass(std::move(path), std::move(csmVertexAttr), shadowMapRast, game_);

    std::string gpath = std::string("./shaders/deferred/GeometryPass.hlsl");
    std::vector<std::pair<const char*, DXGI_FORMAT>> geomVertexAttr{
        std::make_pair("POSITION", DXGI_FORMAT_R32G32B32_FLOAT),
        std::make_pair("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT),
        std::make_pair("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT),
        std::make_pair("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
    };

    CD3D11_RASTERIZER_DESC geomRastDesc = {};
    geomRastDesc.CullMode = D3D11_CULL_NONE;
    geomRastDesc.FillMode = D3D11_FILL_SOLID;
    geometryPass_ = new KatamariGeometryPass(std::move(gpath), std::move(geomVertexAttr), geomRastDesc, game_);


    std::string dirpath = std::string("./shaders/deferred/DirPass.hlsl");
    std::vector<std::pair<const char*, DXGI_FORMAT>> dirLightVertexAttr{
        std::make_pair("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
    };

    CD3D11_RASTERIZER_DESC dirRastDesc = {};
    dirRastDesc.CullMode = D3D11_CULL_NONE;
    dirRastDesc.FillMode = D3D11_FILL_SOLID;

    dirLightPass_ = new KatamariDirectionalLightPass(std::move(dirpath), std::move(dirLightVertexAttr), dirRastDesc, game_);

    std::string pointpath = std::string("./shaders/deferred/PointPass.hlsl");
    std::vector<std::pair<const char*, DXGI_FORMAT>> pointLightVertexAttr{
        std::make_pair("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
    };

    CD3D11_RASTERIZER_DESC pointRastDesc = {};
    pointRastDesc.CullMode = D3D11_CULL_FRONT;
    pointRastDesc.FillMode = D3D11_FILL_SOLID;
    pointLightPass_ = new KatamariPointLightPass(std::move(pointpath), std::move(pointLightVertexAttr), pointRastDesc, game_);

    quad_ = new QuadComponent();
}

KatamariRenderPass::~KatamariRenderPass() {
    delete csmPass_;
    delete geometryPass_;
    delete dirLightPass_;
}

void KatamariRenderPass::Initialize() {
    RenderPass::Initialize();

    csmPass_->Initialize();
    geometryPass_->Initialize();
    dirLightPass_->Initialize();
    pointLightPass_->Initialize();

    D3D11_QUERY_DESC queryDesc;
    ZeroMemory(&queryDesc, sizeof(queryDesc));
    queryDesc.Query = D3D11_QUERY_TIMESTAMP;

    ctx_.GetRenderContext().GetDevice()->CreateQuery(&queryDesc, &startQuery_);
    ctx_.GetRenderContext().GetDevice()->CreateQuery(&queryDesc, &endQuery_);

    D3D11_QUERY_DESC queryDisjoinDesc;
    ZeroMemory(&queryDisjoinDesc, sizeof(queryDisjoinDesc));
    queryDisjoinDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
    ctx_.GetRenderContext().GetDevice()->CreateQuery(&queryDisjoinDesc, &freqQuery_);

    D3D11_BLEND_DESC blendDesc{};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0] =
        D3D11_RENDER_TARGET_BLEND_DESC{
            TRUE,
            D3D11_BLEND_ONE,
            D3D11_BLEND_ONE,
            D3D11_BLEND_OP_ADD,
            D3D11_BLEND_ONE,
            D3D11_BLEND_ONE,
            D3D11_BLEND_OP_ADD,
            D3D11_COLOR_WRITE_ENABLE_ALL
    };

    ctx_.GetRenderContext().GetDevice()->CreateBlendState(&blendDesc, &bs_);

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    ctx_.GetRenderContext().GetDevice()->CreateSamplerState(&sampDesc, &outSampler_);
}

void KatamariRenderPass::Execute() {
    ctx_.GetRenderContext().GetContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    if (!isFetching) {
        ctx_.GetRenderContext().GetContext()->Begin(freqQuery_);
        ctx_.GetRenderContext().GetContext()->End(startQuery_);
    }
    geometryPass_->Execute();
    auto dv = geometryPass_->RenderData().dsv;
    auto rt = geometryPass_->RenderData().rt;

    ctx_.GetRenderContext().GetContext()->OMSetRenderTargets(1, &rt, dv);
    ctx_.GetRenderContext().GetContext()->OMSetBlendState(bs_, nullptr, 0xffffffff);

    ctx_.GetRenderContext().GetContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    csmPass_->Execute();

    ctx_.GetRenderContext().GetContext()->OMSetDepthStencilState(nullptr, 0);
    ctx_.GetRenderContext().GetContext()->OMSetRenderTargets(1, &rt, nullptr);
    ctx_.GetRenderContext().GetContext()->OMSetBlendState(bs_, nullptr, 0xffffffff);

    ctx_.SetViewport(0, 0, ctx_.GetWindow().GetWidth(), ctx_.GetWindow().GetHeight());

    dirLightPass_->Execute();
    //pointLightPass_->Execute();

    ctx_.GetRenderContext().GetContext()->OMSetRenderTargets(1, &rt, dv);
    game_.particles_->Draw();

    ctx_.GetRenderContext().GetContext()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    rt = ctx_.GetWindow().GetRenderTarget();

    //ctx_.GetRenderContext().ActivateDepthStencilState();
    ctx_.GetRenderContext().GetContext()->OMSetDepthStencilState(nullptr, 0);
    ctx_.GetRenderContext().GetContext()->OMSetRenderTargets(1, &rt, nullptr);
    ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
    ctx_.GetRenderContext().GetContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);

    ctx_.SetViewport(0, 0, ctx_.GetWindow().GetWidth(), ctx_.GetWindow().GetHeight());
    auto accum = geometryPass_->RenderData().srvs[4];
    ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(0, 1, &accum);
    ctx_.GetRenderContext().GetContext()->PSSetSamplers(0, 1, &outSampler_);
    ctx_.GetRenderContext().GetContext()->Draw(3, 0);

    // Unbind

    ctx_.GetRenderContext().GetContext()->OMSetDepthStencilState(nullptr, 0);
    ctx_.GetRenderContext().GetContext()->OMSetRenderTargets(0, nullptr, nullptr);
    ctx_.GetRenderContext().GetContext()->RSSetState(nullptr);
    ctx_.GetRenderContext().GetContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);

    ctx_.GetRenderContext().GetContext()->VSSetShader(nullptr, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->PSSetShader(nullptr, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(0, 0, nullptr);
    ctx_.GetRenderContext().GetContext()->PSSetSamplers(0, 0, nullptr);

    if (!isFetching) {
        ctx_.GetRenderContext().GetContext()->End(endQuery_);
        ctx_.GetRenderContext().GetContext()->End(freqQuery_);
    }

    isFetching = true;

    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT ts;

    if (ctx_.GetRenderContext().GetContext()->GetData(freqQuery_, &ts, sizeof(ts), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK) {
        UINT64 startTime = 0;
        ctx_.GetRenderContext().GetContext()->GetData(startQuery_, &startTime, sizeof(startTime), 0);

        UINT64 endTime = 0;
        ctx_.GetRenderContext().GetContext()->GetData(endQuery_, &endTime, sizeof(endTime), 0);
        std::cout << ((float)(endTime - startTime)) / ts.Frequency * 1000.0 << std::endl;

        isFetching = false;
    } 
}

void KatamariRenderPass::DestroyResources() {
    csmPass_->DestroyResources();
    geometryPass_->DestroyResources();
}
