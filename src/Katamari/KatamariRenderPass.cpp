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
#include "KatamariShadowMapPass.h"
#include "KatamariGeometryPass.h"

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
}

KatamariRenderPass::~KatamariRenderPass() {
    delete csmPass_;
    delete geometryPass_;
}

void KatamariRenderPass::Initialize() {
    RenderPass::Initialize();

    csmPass_->Initialize();
    geometryPass_->Initialize();

    D3D11_QUERY_DESC queryDesc;
    ZeroMemory(&queryDesc, sizeof(queryDesc));
    queryDesc.Query = D3D11_QUERY_TIMESTAMP;

    ctx_.GetRenderContext().GetDevice()->CreateQuery(&queryDesc, &startQuery_);
    ctx_.GetRenderContext().GetDevice()->CreateQuery(&queryDesc, &endQuery_);

    D3D11_QUERY_DESC queryDisjoinDesc;
    ZeroMemory(&queryDisjoinDesc, sizeof(queryDisjoinDesc));
    queryDisjoinDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
    ctx_.GetRenderContext().GetDevice()->CreateQuery(&queryDisjoinDesc, &freqQuery_);
}

void KatamariRenderPass::Execute() {
    if (!isFetching) {
        ctx_.GetRenderContext().GetContext()->Begin(freqQuery_);
        ctx_.GetRenderContext().GetContext()->End(startQuery_);
    }
    // csmPass_->Execute();
    geometryPass_->Execute();

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
