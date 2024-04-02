#pragma once
#include "../RenderPass.h"

class KatamariGame;

class KatamariShadowMapPass : public RenderPass
{
private:
    KatamariGame& game_;

    ID3D11Texture2D* texture_;
    ID3D11DepthStencilView* dsv_;
    ID3D11ShaderResourceView* srv_;
    ID3D11DepthStencilState* depthStencilState_;

    ID3D11SamplerState* sampler_;
    ID3D11Buffer* viewProj_;
    ID3D11Buffer* modelBuffer_;
public:
    KatamariShadowMapPass(
        std::string&& shaderPath,
        std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr,
        CD3D11_RASTERIZER_DESC rastState,
        KatamariGame& game
    );

    virtual void Initialize();
    virtual void Execute();
    virtual void DestroyResources();
};

