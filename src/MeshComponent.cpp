#include "MeshComponent.h"

#include "Game.h"
#include "GameComponent.h"
#include "RenderContext.h"
#include <WICTextureLoader.h>

inline std::wstring strToWstr(const std::string& str)
{
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

void MeshComponent::Initialize() {
    D3D11_BUFFER_DESC vertexBufDesc = {};
    vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufDesc.CPUAccessFlags = 0;
    vertexBufDesc.MiscFlags = 0;
    vertexBufDesc.StructureByteStride = 0;
    vertexBufDesc.ByteWidth = sizeof(Vertex) * vertices_.size();

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices_.data();
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&vertexBufDesc, &vertexData, &vb_);

    D3D11_BUFFER_DESC indexBufDesc = {};
    indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufDesc.CPUAccessFlags = 0;
    indexBufDesc.MiscFlags = 0;
    indexBufDesc.StructureByteStride = 0;
    indexBufDesc.ByteWidth = sizeof(int) * indices_.size();

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices_.data();
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&indexBufDesc, &indexData, &ib_);

    auto path = strToWstr(texturePath_.diff);

    // Hack for CreateWICTextureFromFile
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    auto res = DirectX::CreateWICTextureFromFile(
        ctx_.GetRenderContext().GetDevice(),
        path.c_str(),
        &textureData_,
        &texture_);

    path = strToWstr(texturePath_.normal);

    res = DirectX::CreateWICTextureFromFile(
        ctx_.GetRenderContext().GetDevice(),
        path.c_str(),
        &normalData_,
        &normal_);

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    ctx_.GetRenderContext().GetDevice()->CreateSamplerState(&sampDesc, &sampler_);
}

void MeshComponent::Update(float deltaTime) {
}

void MeshComponent::Draw() {
    UINT strides[] = { sizeof(Vertex) };
    UINT offsets[] = { 0 };

    ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(ib_, DXGI_FORMAT_R32_UINT, 0);
    ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &vb_, strides, offsets);
    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(0, 1, &texture_);
    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(1, 1, &normal_);
    ctx_.GetRenderContext().GetContext()->PSSetSamplers(0, 1, &sampler_);

    ctx_.GetRenderContext().GetContext()->DrawIndexed(indices_.size(), 0, 0);
}

void MeshComponent::Reload() {
}

void MeshComponent::DestroyResources() {
    vb_->Release();
    ib_->Release();
    texture_->Release();
    textureData_->Release();
    sampler_->Release();
}