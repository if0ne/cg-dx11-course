#pragma once
#include "../RenderPass.h"

class KatamariGame;
class QuadComponent;

class KatamariDirectionalLightPass : public RenderPass
{
private:
    KatamariGame& game_;

    ID3D11Buffer* screenSizeBuffer_;
    ID3D11Buffer* cascadeBuffer_;
    ID3D11Buffer* dirLightBuffer_;
    ID3D11Buffer* ambientLightBuffer_;
    ID3D11Buffer* viewPosBuffer_;

    QuadComponent* quad_;
public:
    KatamariDirectionalLightPass(
        std::string&& shaderPath,
        std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr,
        CD3D11_RASTERIZER_DESC rastState,
        KatamariGame& game
    );

    ~KatamariDirectionalLightPass();

    virtual void Initialize();
    virtual void Execute();
    virtual void DestroyResources();
};

