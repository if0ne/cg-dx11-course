#pragma once
#include "../RenderPass.h"

class KatamariGame;
class SphereComponent;

struct ScreenToViewParams
{
    DirectX::SimpleMath::Matrix InverseProjectionView;
    DirectX::SimpleMath::Vector4 ScreenDimensions;
};

class KatamariPointLightPass : public RenderPass
{
private:
    KatamariGame& game_;

    ID3D11Buffer* modelBuffer_;
    ID3D11Buffer* pointLightBuffer_;
    ID3D11Buffer* viewPosBuffer_;
    ID3D11Buffer* cameraBuffer_;
    ID3D11Buffer* svBuffer_;

    ID3D11RasterizerState* insideState_;
    ID3D11DepthStencilState* depthState_;
    ID3D11DepthStencilState* insideDepthState_;

    SphereComponent* sphere_;
public:
    KatamariPointLightPass(
        std::string&& shaderPath,
        std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr,
        CD3D11_RASTERIZER_DESC rastState,
        KatamariGame& game
    );

    ~KatamariPointLightPass();

    virtual void Initialize();
    virtual void Execute();
    virtual void DestroyResources();
};

