#pragma once

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <SimpleMath.h>

#include <string>
#include <vector>

class Game;

class RenderPass
{
protected:
    Game& ctx_;

    ID3D11InputLayout* layout_;
    ID3D11VertexShader* vertexShader_;
    ID3D11PixelShader* pixelShader_;

    ID3DBlob* vertexBC_;
    ID3DBlob* pixelBC_;

    ID3D11RasterizerState* rastState_;

    std::vector<ID3D11Buffer*> constBuffers_;

    std::string shaderPath_;
    std::vector<std::pair<const char*, DXGI_FORMAT>> vertexAttr_;
public:
    RenderPass(
        std::string&& shaderPath,
        std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr
    );

    virtual void Initialize();
    virtual void Execute() = 0;

    ID3D11Buffer* CreateBuffer(size_t size);
    void UpdateBuffer(ID3D11Buffer* buffer, void* data, size_t size);

    virtual void DestroyResources();
};

