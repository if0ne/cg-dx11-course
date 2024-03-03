#pragma once
#include <dxgi1_2.h>
#include <d3d.h>
#include <d3d11.h>

class RenderContext 
{
private:
    ID3D11Device* device_;
    ID3D11DeviceContext* context_;
    IDXGIFactory2* factory_;

    ID3D11DepthStencilState* depthStencilState_;

public:
    RenderContext() {}

    void Initialize();
    void DestroyResources();

    void ActivateDepthStencilState();

    ID3D11Device* GetDevice() {
        return device_;
    }

    ID3D11DeviceContext* GetContext() {
        return context_;
    }

    IDXGIFactory2* GetFactory() {
        return factory_;
    }
};