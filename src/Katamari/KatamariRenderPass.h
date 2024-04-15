#pragma once
#include "../RenderPass.h"

class KatamariGame;
class KatamariCSMPass;
class KatamariShadowMapPass;
class KatamariGeometryPass;

class KatamariRenderPass : public RenderPass
{
private:
    KatamariGame& game_;

    KatamariCSMPass* csmPass_;
    KatamariGeometryPass* geometryPass_;

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

