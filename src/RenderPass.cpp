#include "RenderPass.h"

#include "Game.h"
#include "RenderContext.h"
#include "Window.h"

inline std::wstring strToWstr(const std::string& str)
{
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

RenderPass::RenderPass(
    std::string&& shaderPath,
    std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr
) : 
    shaderPath_(shaderPath),
    vertexAttr_(vertexAttr),
    ctx_(Game::GetSingleton()) {
}

void RenderPass::Initialize() {
    D3DCompileFromFile(
        strToWstr(shaderPath_).c_str(),
        nullptr,
        nullptr,
        "VSMain",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
        0,
        &vertexBC_,
        nullptr
    );
    
    D3DCompileFromFile(
        strToWstr(shaderPath_).c_str(),
        nullptr,
        nullptr,
        "PSMain",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &pixelBC_,
        nullptr
    );

    ctx_.GetRenderContext().GetDevice()->CreateVertexShader(
        vertexBC_->GetBufferPointer(),
        vertexBC_->GetBufferSize(),
        nullptr, &vertexShader_
    );

    ctx_.GetRenderContext().GetDevice()->CreatePixelShader(
        pixelBC_->GetBufferPointer(),
        pixelBC_->GetBufferSize(),
        nullptr, &pixelShader_
    );

    std::vector<D3D11_INPUT_ELEMENT_DESC> vertexAttrDesc;

    for (auto& attr : vertexAttr_) {
        vertexAttrDesc.push_back(
            D3D11_INPUT_ELEMENT_DESC{
                attr.first,
                0,
                attr.second,
                0,
                D3D11_APPEND_ALIGNED_ELEMENT,
                D3D11_INPUT_PER_VERTEX_DATA,
                0
            });
    }

    ctx_.GetRenderContext().GetDevice()->CreateInputLayout(
        vertexAttrDesc.data(),
        vertexAttrDesc.size(),
        vertexBC_->GetBufferPointer(),
        vertexBC_->GetBufferSize(),
        &layout_
    );

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID;

    ctx_.GetRenderContext().GetDevice()->CreateRasterizerState(&rastDesc, &rastState_);
}

ID3D11Buffer* RenderPass::CreateBuffer(size_t size) {
    auto index = constBuffers_.size();
    constBuffers_.push_back(nullptr);

    D3D11_BUFFER_DESC constBufDesc = {};
    constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
    constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constBufDesc.MiscFlags = 0;
    constBufDesc.StructureByteStride = 0;
    constBufDesc.ByteWidth = size;

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&constBufDesc, nullptr, &constBuffers_[index]);

    return constBuffers_[index];
}

void RenderPass::UpdateBuffer(ID3D11Buffer* buffer, void* data, size_t size) {
    D3D11_MAPPED_SUBRESOURCE res = {};
    ctx_.GetRenderContext().GetContext()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, data, size);
    ctx_.GetRenderContext().GetContext()->Unmap(buffer, 0);
}

void RenderPass::AddSubpass(RenderPass* pass) {
    subpasses_.push_back(pass);
}

void RenderPass::DestroyResources() {
    layout_->Release();
    vertexShader_->Release();
    pixelShader_->Release();
    vertexBC_->Release();
    pixelBC_->Release();
    rastState_->Release();

    for (auto& buffer : constBuffers_) {
        buffer->Release();
    }

    for (auto& pass : subpasses_) {
        pass->DestroyResources();
    }
}
