#pragma once
#include "../RenderPass.h"
#include "../CascadedShadowMap.h"

class KatamariGame;

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
public:
    KatamariCSMPass(
        std::string&& shaderPath,
        std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr,
        KatamariGame& game
    );

    virtual void Initialize();
    virtual void Execute();
    virtual void DestroyResources();
};

