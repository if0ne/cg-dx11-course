#include "RenderContext.h"

void RenderContext::Initialize() {
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, 1, D3D11_SDK_VERSION, &device_, nullptr, &context_);

    IDXGIDevice* dxgiDevice = nullptr;
    device_->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));

    IDXGIAdapter* adapter = nullptr;
    dxgiDevice->GetAdapter(&adapter);
    adapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&factory_));

    adapter->Release();
    dxgiDevice->Release();
}

void RenderContext::DestroyResources() {
    factory_->Release();
    context_->Release();
    device_->Release();
}