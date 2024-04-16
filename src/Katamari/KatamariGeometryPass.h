#pragma once
#include "../RenderPass.h"

class KatamariGame;

struct GeometryPassData {
    ID3D11ShaderResourceView* srvs[4];
};

class KatamariGeometryPass : public RenderPass
{
private:
    KatamariGame& game_;

    ID3D11Buffer* wvpBuffer_;
    ID3D11Buffer* modelBuffer_;
    ID3D11Buffer* materialBuffer_;

    ID3D11Texture2D* depthTexture_;
    ID3D11DepthStencilView* dsv_;
    ID3D11DepthStencilState* depthStencilState_;

    // 1 - diffuse
    // 2 - normal
    // 3 - material
    // 4 - world pos
    ID3D11Texture2D* textures_[4];
    ID3D11RenderTargetView* rtvs_[4];
    ID3D11ShaderResourceView* srvs_[4];
public:
    KatamariGeometryPass(
        std::string&& shaderPath,
        std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr,
        CD3D11_RASTERIZER_DESC rastState,
        KatamariGame& game
    );

    ~KatamariGeometryPass();

    virtual void Initialize();
    virtual void Execute();
    virtual void DestroyResources();

    GeometryPassData RenderData() {
        return {
            { srvs_[0], srvs_[1], srvs_[2], srvs_[3] }
        };
    }
};

