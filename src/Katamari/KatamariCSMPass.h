#pragma once
#include "../RenderPass.h"
#include "../CascadedShadowMap.h"

class KatamariGame;

struct CSMRenderData {
    ID3D11ShaderResourceView* srv;
    ID3D11SamplerState* sampler;
    CascadedShadowMapData cascades;
};

class KatamariCSMPass : public RenderPass
{
private:
    KatamariGame& game_;

    CascadedShadowMap csm_;

    ID3D11Texture2D* textureArray_;
    ID3D11DepthStencilView* dsv_[4];
    ID3D11ShaderResourceView* srv_;
    ID3D11DepthStencilState* depthStencilState_;

    ID3D11SamplerState* sampler_;
    ID3D11Buffer* cascadeBuffer_;
    ID3D11Buffer* modelBuffer_;
public:
    KatamariCSMPass(
        std::string&& shaderPath,
        std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr,
        KatamariGame& game
    );

    virtual void Initialize();
    virtual void Execute();
    virtual void DestroyResources();

    CSMRenderData RenderData();
};

