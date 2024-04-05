#pragma once
#include "../RenderPass.h"

class KatamariGame;
class KatamariCSMPass;
class KatamariShadowMapPass;

class KatamariRenderPass : public RenderPass
{
private:
    KatamariGame& game_;

    ID3D11Buffer* wvpBuffer_;
    ID3D11Buffer* modelBuffer_;
    ID3D11Buffer* dirLightBuffer_;
    ID3D11Buffer* pointLightBuffer_;
    ID3D11Buffer* ambientLightBuffer_;
    ID3D11Buffer* materialBuffer_;
    ID3D11Buffer* viewPosBuffer_;
    ID3D11Buffer* cascadeBuffer_;

    KatamariCSMPass* csmPass_;
    KatamariShadowMapPass* sm_;

    ID3D11Query* startQuery_;
    ID3D11Query* endQuery_;
    ID3D11Query* freqQuery_;
    bool isFetching = false;
public:
    KatamariRenderPass(
        std::string&& shaderPath,
        std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr,
        CD3D11_RASTERIZER_DESC rastState,
        KatamariGame& game
    );

    ~KatamariRenderPass();

    virtual void Initialize();
    virtual void Execute();
    virtual void DestroyResources();
};

