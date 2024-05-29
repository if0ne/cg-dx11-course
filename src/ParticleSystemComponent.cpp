#include "ParticleSystemComponent.h"
#include "Game.h"
#include "RenderContext.h"
#include <d3dcompiler.h>
#include <iostream>
#include <WICTextureLoader.h>
#include <random>

using namespace DirectX::SimpleMath;

using int4 = struct SortConstants
{
    int x, y, z, w;
};

int align(int value, int alignment)
{
    return (value + (alignment - 1)) & ~(alignment - 1);
}

void ParticleSystemComponent::Initialize()
{
    emitterProps_ = {
        {0.0, 0.0, 0.0},
        10,
        5.0f
    };

    currentAliveBuffer_ = 0;

    {
        ID3DBlob* errorBlob = nullptr;
        HRESULT res = D3DCompileFromFile(L"./shaders/particles/Emit.hlsl",
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "CSMain",
            "cs_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
            0,
            &emitBlobCs_,
            &errorBlob);

        if (FAILED(res)) {
            if (errorBlob) {
                char* compileErrors = (char*)(errorBlob->GetBufferPointer());

                std::cout << compileErrors << std::endl;

                assert(false);
            }
        }


        ctx_.GetRenderContext().GetDevice()->CreateComputeShader(
            emitBlobCs_->GetBufferPointer(),
            emitBlobCs_->GetBufferSize(),
            nullptr, &emitCs_);
    }

    {
        ID3DBlob* errorBlob = nullptr;
        HRESULT res = D3DCompileFromFile(L"./shaders/particles/Simulate.hlsl",
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "CSMain",
            "cs_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
            0,
            &simulateBlobCs_,
            &errorBlob);

        if (FAILED(res)) {
            if (errorBlob) {
                char* compileErrors = (char*)(errorBlob->GetBufferPointer());

                std::cout << compileErrors << std::endl;

                assert(false);
            }
        }


        ctx_.GetRenderContext().GetDevice()->CreateComputeShader(
            simulateBlobCs_->GetBufferPointer(),
            simulateBlobCs_->GetBufferSize(),
            nullptr, &simulateCs_);
    }

    {
        ID3DBlob* errorBlob = nullptr;
        HRESULT res = D3DCompileFromFile(L"./shaders/particles/Reset.hlsl",
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "CSMain",
            "cs_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
            0,
            &resetBlboCs_,
            &errorBlob);

        if (FAILED(res)) {
            if (errorBlob) {
                char* compileErrors = (char*)(errorBlob->GetBufferPointer());

                std::cout << compileErrors << std::endl;

                assert(false);
            }
        }


        ctx_.GetRenderContext().GetDevice()->CreateComputeShader(
            resetBlboCs_->GetBufferPointer(),
            resetBlboCs_->GetBufferSize(),
            nullptr, &resetCs_);
    }

    {
        D3DCompileFromFile(
            L"./shaders/particles/Render.hlsl",
            nullptr,
            nullptr,
            "VSMain",
            "vs_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
            0,
            &vertexBlob_,
            nullptr
        );

        D3DCompileFromFile(
            L"./shaders/particles/Render.hlsl",
            nullptr,
            nullptr,
            "PSMain",
            "ps_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
            0,
            &pixelBlob_,
            nullptr
        );

        ID3DBlob* errorBlob = nullptr;
        D3DCompileFromFile(
            L"./shaders/particles/Render.hlsl",
            nullptr,
            nullptr,
            "GSMain",
            "gs_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
            0,
            &geometryBlob_,
            nullptr
        );

        ctx_.GetRenderContext().GetDevice()->CreateVertexShader(
            vertexBlob_->GetBufferPointer(),
            vertexBlob_->GetBufferSize(),
            nullptr, &vertexShader_
        );

        ctx_.GetRenderContext().GetDevice()->CreatePixelShader(
            pixelBlob_->GetBufferPointer(),
            pixelBlob_->GetBufferSize(),
            nullptr, &pixelShader_
        );

        ctx_.GetRenderContext().GetDevice()->CreateGeometryShader(
            geometryBlob_->GetBufferPointer(),
            geometryBlob_->GetBufferSize(),
            nullptr, &geometryShader_
        );
    }

    {
        D3D11_BLEND_DESC blendDesc{};
        blendDesc.AlphaToCoverageEnable = false;
        blendDesc.IndependentBlendEnable = false;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        blendDesc.RenderTarget[0].BlendEnable = true;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        ctx_.GetRenderContext().GetDevice()->CreateBlendState(&blendDesc, &blendState_);
    }

    {
        D3D11_BUFFER_DESC constBufDesc = {};
        constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
        constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constBufDesc.MiscFlags = 0;
        constBufDesc.StructureByteStride = 0;
        constBufDesc.ByteWidth = sizeof(EmitterProperties);

        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&constBufDesc, nullptr, &emitterBuffer_);
    }

    {
        D3D11_BUFFER_DESC constBufDesc = {};
        constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
        constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constBufDesc.MiscFlags = 0;
        constBufDesc.StructureByteStride = 0;
        constBufDesc.ByteWidth = sizeof(DirectX::SimpleMath::Vector4);

        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&constBufDesc, nullptr, &frameTimeBuffer_);
    }

    {
        D3D11_BUFFER_DESC constBufDesc = {};
        constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
        constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constBufDesc.MiscFlags = 0;
        constBufDesc.StructureByteStride = 0;
        constBufDesc.ByteWidth = 2 * sizeof(DirectX::SimpleMath::Matrix);

        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&constBufDesc, nullptr, &viewProjBuffer_);
    }

    {
        auto res = DirectX::CreateWICTextureFromFile(
            ctx_.GetRenderContext().GetDevice(),
            L"./assets/whiteCircle.png",
            &albedoTexture_,
            &albedoSrv_);
    }

    {
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

    {
        const int WidthHeight = 1024;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> distrib(-1, 1);

        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = WidthHeight;
        desc.Height = WidthHeight;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        std::vector<float> values(WidthHeight* WidthHeight * 4);
        float* ptr = values.data();
        for (UINT i = 0; i < desc.Width * desc.Height; i++)
        {
            ptr[0] = distrib(gen);
            ptr[1] = distrib(gen);
            ptr[2] = distrib(gen);
            ptr[3] = distrib(gen);
            ptr += 4;
        }

        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = values.data();
        data.SysMemPitch = desc.Width * 16;
        data.SysMemSlicePitch = 0;

        ctx_.GetRenderContext().GetDevice()->CreateTexture2D(&desc, &data, (ID3D11Texture2D**)&randomTexture_);

        D3D11_SHADER_RESOURCE_VIEW_DESC srv;
        srv.Format = desc.Format;
        srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv.Texture2D.MipLevels = 1;
        srv.Texture2D.MostDetailedMip = 0;

        ctx_.GetRenderContext().GetDevice()->CreateShaderResourceView(randomTexture_, &srv, &randomSrv_);
    }

    {
        CD3D11_RASTERIZER_DESC rast
        {
            D3D11_FILL_SOLID,
            D3D11_CULL_FRONT,
            TRUE,
            D3D11_DEFAULT_DEPTH_BIAS,
            D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
            D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
            TRUE,
            FALSE,
            FALSE,
            FALSE
        };

        ctx_.GetRenderContext().GetDevice()->CreateRasterizerState(&rast, &rastState_);
    }

    {
        D3D11_BUFFER_DESC cbDesc;
        ZeroMemory(&cbDesc, sizeof(cbDesc));
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbDesc.ByteWidth = sizeof(int4);
        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&cbDesc, nullptr, &dispatchInfoBuffer_);
    }

    {
        ID3DBlob* pBlob;
        ID3DBlob* pErrorBlob;

        // Step sort shader
        HRESULT hr = D3DCompileFromFile(L"./shaders/particles/SortStep2.hlsl", nullptr, nullptr, "BitonicSortStep",
            "cs_5_0", 0, 0, &pBlob, &pErrorBlob);
        if (FAILED(hr))
        {
            if (pErrorBlob != nullptr)
                OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        }

        ctx_.GetRenderContext().GetDevice()->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &sortStep_);
    }

    {
        ID3DBlob* pBlob;
        ID3DBlob* pErrorBlob;

        const D3D10_SHADER_MACRO innerDefines[2] = { {"SORT_SIZE", "512"}, {nullptr, 0} };
        HRESULT hr = D3DCompileFromFile(L"./shaders/particles/SortInner.hlsl", innerDefines, nullptr, "BitonicInnerSort",
            "cs_5_0", 0, 0, &pBlob, &pErrorBlob);
        if (FAILED(hr))
        {
            if (pErrorBlob != NULL)
                OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        }
        ctx_.GetRenderContext().GetDevice()->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &sortInner512_);
    }

    {
        ID3DBlob* pBlob;
        ID3DBlob* pErrorBlob;

        const D3D10_SHADER_MACRO cs512Defines[2] = { {"SORT_SIZE", "512"}, {nullptr, 0} };
        HRESULT hr = D3DCompileFromFile(L"./shaders/particles/Sort512.hlsl", cs512Defines, nullptr, "BitonicSortLDS", "cs_5_0", 0,
            0, &pBlob, &pErrorBlob);
        if (FAILED(hr))
        {
            if (pErrorBlob != nullptr)
                OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        }
        ctx_.GetRenderContext().GetDevice()->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &sort512_);
    }

    {
        ID3DBlob* pBlob;
        ID3DBlob* pErrorBlob;

        const D3D10_SHADER_MACRO cs512Defines[2] = { {"SORT_SIZE", "512"}, {nullptr, 0} };
        HRESULT  hr = D3DCompileFromFile(L"./shaders/particles/InitSort.hlsl", nullptr, nullptr, "InitDispatchArgs",
            "cs_5_0", 0, 0, &pBlob, &pErrorBlob);
        if (FAILED(hr))
        {
            if (pErrorBlob != nullptr)
                OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        }
        ctx_.GetRenderContext().GetDevice()->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &sortInitArgs_);
    }

    {
        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = MAX_PARTICLE_COUNT * sizeof(NewParticle);
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride = sizeof(NewParticle);

        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&bufferDesc, nullptr, &particleBuffer_);

        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
        srv_desc.Format = DXGI_FORMAT_UNKNOWN;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srv_desc.Buffer.ElementOffset = 0;
        srv_desc.Buffer.NumElements = MAX_PARTICLE_COUNT;

        ctx_.GetRenderContext().GetDevice()->CreateShaderResourceView(particleBuffer_, &srv_desc, &particleBufferSrv_);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.Format = DXGI_FORMAT_UNKNOWN;
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uav_desc.Buffer.FirstElement = 0;
        uav_desc.Buffer.Flags = 0;
        uav_desc.Buffer.NumElements = MAX_PARTICLE_COUNT;
        ctx_.GetRenderContext().GetDevice()->CreateUnorderedAccessView(particleBuffer_, &uav_desc, &particleBufferUav_);
    }

    {
        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = MAX_PARTICLE_COUNT * sizeof(uint32_t);
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride = sizeof(uint32_t);

        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&bufferDesc, nullptr, &deadBuffer_);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.Format = DXGI_FORMAT_UNKNOWN;
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uav_desc.Buffer.FirstElement = 0;
        uav_desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
        uav_desc.Buffer.NumElements = MAX_PARTICLE_COUNT;
        ctx_.GetRenderContext().GetDevice()->CreateUnorderedAccessView(deadBuffer_, &uav_desc, &deadBufferUav_);
    }

    {
        struct ParticleIndexElement
        {
            float distance;
            float index;
        };

        D3D11_BUFFER_DESC aliveIndexBufferDesc{};
        aliveIndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        aliveIndexBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        aliveIndexBufferDesc.CPUAccessFlags = 0;
        aliveIndexBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        aliveIndexBufferDesc.ByteWidth = sizeof(ParticleIndexElement) * MAX_PARTICLE_COUNT;
        aliveIndexBufferDesc.StructureByteStride = sizeof(ParticleIndexElement);

        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&aliveIndexBufferDesc, nullptr, &aliveBuffer_[0]);
        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&aliveIndexBufferDesc, nullptr, &aliveBuffer_[1]);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.Format = DXGI_FORMAT_UNKNOWN;
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uav_desc.Buffer.FirstElement = 0;
        uav_desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
        uav_desc.Buffer.NumElements = MAX_PARTICLE_COUNT;
        ctx_.GetRenderContext().GetDevice()->CreateUnorderedAccessView(aliveBuffer_[0], &uav_desc, &aliveBufferUav_[0]);
        ctx_.GetRenderContext().GetDevice()->CreateUnorderedAccessView(aliveBuffer_[1], &uav_desc, &aliveBufferUav_[1]);

        D3D11_SHADER_RESOURCE_VIEW_DESC aliveIndexSRVDesc{};
        aliveIndexSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
        aliveIndexSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        aliveIndexSRVDesc.Buffer.ElementOffset = 0;
        aliveIndexSRVDesc.Buffer.NumElements = MAX_PARTICLE_COUNT;

        ctx_.GetRenderContext().GetDevice()->CreateShaderResourceView(aliveBuffer_[0], &aliveIndexSRVDesc, &aliveBufferSrv_[0]);
        ctx_.GetRenderContext().GetDevice()->CreateShaderResourceView(aliveBuffer_[1], &aliveIndexSRVDesc, &aliveBufferSrv_[1]);

        uav_desc.Buffer.Flags = 0;
        ctx_.GetRenderContext().GetDevice()->CreateUnorderedAccessView(aliveBuffer_[0], &uav_desc, &aliveBufferSortingUav_[0]);
        ctx_.GetRenderContext().GetDevice()->CreateUnorderedAccessView(aliveBuffer_[1], &uav_desc, &aliveBufferSortingUav_[1]);
    }

    {
        struct DeadListCountConstantBuffer
        {
            UINT nbDeadParticles;

            UINT padding[3];
        };

        D3D11_BUFFER_DESC desc{};  
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;
        desc.ByteWidth = sizeof(DeadListCountConstantBuffer);

        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&desc, nullptr, &deadCounterBuffer_);
        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&desc, nullptr, &aliveCounterBuffer_);
    }

    {
        D3D11_BUFFER_DESC desc{};
        ZeroMemory(&desc, sizeof(desc));
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
        desc.ByteWidth = 3 * sizeof(UINT);
        desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;

        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&desc, nullptr, &indirectBuffer_[0]);
        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&desc, nullptr, &indirectBuffer_[1]);

        D3D11_UNORDERED_ACCESS_VIEW_DESC indirectDispatchArgsUAVDesc{};
        indirectDispatchArgsUAVDesc.Format = DXGI_FORMAT_R32_UINT;
        indirectDispatchArgsUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        indirectDispatchArgsUAVDesc.Buffer.FirstElement = 0;
        indirectDispatchArgsUAVDesc.Buffer.NumElements = 3;
        indirectDispatchArgsUAVDesc.Buffer.Flags = 0;

        ctx_.GetRenderContext().GetDevice()->CreateUnorderedAccessView(indirectBuffer_[0], &indirectDispatchArgsUAVDesc, &indirectBufferUav_[0]);
        ctx_.GetRenderContext().GetDevice()->CreateUnorderedAccessView(indirectBuffer_[1], &indirectDispatchArgsUAVDesc, &indirectBufferUav_[1]);
    }

    {
        D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc{};

        depthStencilStateDesc.DepthEnable = TRUE;
        depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        depthStencilStateDesc.StencilEnable = FALSE;

        ctx_.GetRenderContext().GetDevice()->CreateDepthStencilState(&depthStencilStateDesc, &depthState_);
    }

    {
        D3D11_BUFFER_DESC indirectDrawArgsBuffer{};
        indirectDrawArgsBuffer.Usage = D3D11_USAGE_DEFAULT;
        indirectDrawArgsBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
        indirectDrawArgsBuffer.ByteWidth = 5 * sizeof(UINT);
        indirectDrawArgsBuffer.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;

        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&indirectDrawArgsBuffer, nullptr, &indirectDrawBuffer_);

        D3D11_UNORDERED_ACCESS_VIEW_DESC indirectDrawArgsUAVDesc{};
        indirectDrawArgsUAVDesc.Format = DXGI_FORMAT_R32_UINT;
        indirectDrawArgsUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        indirectDrawArgsUAVDesc.Buffer.FirstElement = 0;
        indirectDrawArgsUAVDesc.Buffer.NumElements = 5;
        indirectDrawArgsUAVDesc.Buffer.Flags = 0;

        ctx_.GetRenderContext().GetDevice()->CreateUnorderedAccessView(indirectDrawBuffer_, &indirectDrawArgsUAVDesc, &indirectDrawBufferUav_);
    }

    Reload();
}

void ParticleSystemComponent::Update(float deltaTime)
{
}

void ParticleSystemComponent::Draw()
{
    ID3D11RenderTargetView* rtv = nullptr;
    ID3D11DepthStencilView* dsv = nullptr;
    ctx_.GetRenderContext().GetContext()->OMGetRenderTargets(1, &rtv, &dsv);
    ctx_.GetRenderContext().GetContext()->OMSetRenderTargets(0, nullptr, nullptr);

    Emit();
    Simulate();

    ctx_.GetRenderContext().GetContext()->OMSetRenderTargets(1, &rtv, dsv);

    ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->GSSetShader(geometryShader_, nullptr, 0);

    ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
    ctx_.GetRenderContext().GetContext()->OMSetDepthStencilState(depthState_, 0);

    ID3D11Buffer* nullVertexBuffer = nullptr;
    UINT stride = 0;
    UINT offset = 0;
    ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &nullVertexBuffer, &stride, &offset);
    ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

    ctx_.GetRenderContext().GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    ctx_.GetRenderContext().GetContext()->OMSetBlendState(blendState_, nullptr, 0xffffffff);

    DirectX::SimpleMath::Matrix matrices[2] = { camera_->View(), camera_->Projection() };

    UpdateBuffer(viewProjBuffer_, &matrices, 2 * sizeof(DirectX::SimpleMath::Matrix));

    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(1, 1, &viewProjBuffer_);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(3, 1, &aliveCounterBuffer_);

    ctx_.GetRenderContext().GetContext()->VSSetShaderResources(3, 1, &particleBufferSrv_);
    ctx_.GetRenderContext().GetContext()->VSSetShaderResources(4, 1, &aliveBufferSrv_[currentAliveBuffer_]);

    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(0, 1, &albedoSrv_);
    ctx_.GetRenderContext().GetContext()->PSSetSamplers(0, 1, &sampler_);

    ctx_.GetRenderContext().GetContext()->GSSetConstantBuffers(1, 1, &viewProjBuffer_);

    ctx_.GetRenderContext().GetContext()->DrawInstancedIndirect(indirectDrawBuffer_, 0);
    //ctx_.GetRenderContext().GetContext()->Draw(MAX_PARTICLE_COUNT, 0);

    ctx_.GetRenderContext().GetContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    ctx_.GetRenderContext().GetContext()->GSSetShader(nullptr, nullptr, 0);
}

void ParticleSystemComponent::Reload()
{
    UINT initialCount[] = { 0 };
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(0, 1, &deadBufferUav_, initialCount);
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(1, 1, &particleBufferUav_, initialCount);
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(2, 1, &indirectBufferUav_[0], initialCount);
    int numThreadGroups = align(emitterProps_.maxNumToEmit, 256) / 256;
    ctx_.GetRenderContext().GetContext()->CSSetShader(resetCs_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->Dispatch(numThreadGroups, 1, 1);
}

void ParticleSystemComponent::DestroyResources()
{
}

void ParticleSystemComponent::Sort()
{
    /*ID3D11UnorderedAccessView* prevUAV = nullptr;
    ctx_.GetRenderContext().GetContext()->CSGetUnorderedAccessViews(0, 1, &prevUAV);

    ID3D11Buffer* prevCBs[] = { nullptr, nullptr };
    ctx_.GetRenderContext().GetContext()->CSGetConstantBuffers(0, ARRAYSIZE(prevCBs), prevCBs);

    ID3D11Buffer* cbs[] = { aliveCounterBuffer_, dispatchInfoBuffer_ };
    ctx_.GetRenderContext().GetContext()->CSSetConstantBuffers(0, ARRAYSIZE(cbs), cbs);

    //ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(0, 1, &indirectSortArgsBufferUAV_, nullptr);

    ctx_.GetRenderContext().GetContext()->CSSetShader(sortInitArgs_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->Dispatch(1, 1, 1);


    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(0, 1, &aliveListUAV, nullptr);

    bool bDone = SortInitial(MAX_PARTICLE_COUNT);

    int presorted = 512;
    while (!bDone)
    {
        bDone = SortIncremental(presorted, MAX_PARTICLE_COUNT);
        presorted *= 2;
    }

    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(0, 1, &prevUAV, nullptr);
    ctx_.GetRenderContext().GetContext()->CSSetConstantBuffers(0, ARRAYSIZE(prevCBs), prevCBs);

    if (prevUAV)
        prevUAV->Release();

    for (size_t i = 0; i < ARRAYSIZE(prevCBs); i++)
        if (prevCBs[i])
            prevCBs[i]->Release();*/
}

void ParticleSystemComponent::Emit()
{
    ID3D11UnorderedAccessView* uavs[] = { particleBufferUav_, deadBufferUav_, aliveBufferUav_[currentAliveBuffer_], indirectBufferUav_[currentAliveBuffer_] };
    UINT initialCounts[] = { (UINT)-1, (UINT)-1, (UINT)-1, (UINT)-1 };
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(0, std::size(uavs), uavs, initialCounts);

    ID3D11Buffer* buffers[] = { frameTimeBuffer_, emitterBuffer_, deadCounterBuffer_, viewProjBuffer_ };
    auto time = DirectX::SimpleMath::Vector4(ctx_.GetDeltaTime(), 0.0, 0.0, 0.0);
    DirectX::SimpleMath::Matrix matrices[2] = {camera_->View(), camera_->Projection()};

    UpdateBuffer(frameTimeBuffer_, &time, sizeof(DirectX::SimpleMath::Vector4));
    UpdateBuffer(emitterBuffer_, &emitterProps_, sizeof(EmitterProperties));
    UpdateBuffer(viewProjBuffer_, &matrices, 2 * sizeof(DirectX::SimpleMath::Matrix));
    ctx_.GetRenderContext().GetContext()->CSSetConstantBuffers(0, ARRAYSIZE(buffers), buffers);

    ID3D11ShaderResourceView* srvs[] = { randomSrv_ };
    ctx_.GetRenderContext().GetContext()->CSSetShaderResources(0, 1, srvs);
    ctx_.GetRenderContext().GetContext()->CSSetSamplers(0, 1, &sampler_);

    ctx_.GetRenderContext().GetContext()->CSSetShader(emitCs_, nullptr, 0);

    ctx_.GetRenderContext().GetContext()->CopyStructureCount(deadCounterBuffer_, 0, deadBufferUav_);

    int numThreadGroups = align(emitterProps_.maxNumToEmit, X_NUMTHREADS) / X_NUMTHREADS;
    ctx_.GetRenderContext().GetContext()->Dispatch(numThreadGroups, 1, 1);
}

void ParticleSystemComponent::Simulate()
{
    ctx_.GetRenderContext().GetContext()->CopyStructureCount(aliveCounterBuffer_, 0, aliveBufferUav_[currentAliveBuffer_]);

    UINT initialCount[] = { (UINT)-1 };

    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(0, 1, &indirectDrawBufferUav_, initialCount);
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(2, 1, &deadBufferUav_, initialCount);
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(3, 1, &particleBufferUav_, initialCount);
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(4, 1, &aliveBufferUav_[currentAliveBuffer_], initialCount);
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(5, 1, &indirectBufferUav_[(currentAliveBuffer_ + 1) % 2], initialCount);

    initialCount[0] = 0;
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(1, 1, &aliveBufferUav_[(currentAliveBuffer_ + 1) % 2], initialCount);

    currentAliveBuffer_ = (currentAliveBuffer_ + 1) % 2;

    ctx_.GetRenderContext().GetContext()->CSSetShader(simulateCs_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->Dispatch(align(MAX_PARTICLE_COUNT, 256) / 256, 1, 1);

    ctx_.GetRenderContext().GetContext()->CopyStructureCount(aliveCounterBuffer_, 0, aliveBufferUav_[(currentAliveBuffer_ + 1) % 2]);

    ID3D11UnorderedAccessView* const ruav[1] = { nullptr };
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(0, 1, ruav, nullptr);
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(1, 1, ruav, nullptr);
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(2, 1, ruav, nullptr);
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(3, 1, ruav, nullptr);
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(4, 1, ruav, nullptr);
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(5, 1, ruav, nullptr);
}

void ParticleSystemComponent::UpdateBuffer(ID3D11Buffer* buffer, void* data, size_t size) {
    D3D11_MAPPED_SUBRESOURCE res = {};
    ctx_.GetRenderContext().GetContext()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, data, size);
    ctx_.GetRenderContext().GetContext()->Unmap(buffer, 0);
}

bool ParticleSystemComponent::SortInitial(unsigned int maxSize)
{
    bool bDone = true;

    unsigned int numThreadGroups = ((maxSize - 1) >> 9) + 1;

    if (numThreadGroups > 1) bDone = false;

    ctx_.GetRenderContext().GetContext()->CSSetShader(sort512_, nullptr, 0);
    //ctx_.GetRenderContext().GetContext()->DispatchIndirect(indirectSortArgsBuffer_, 0);

    return bDone;
}

bool ParticleSystemComponent::SortIncremental(unsigned int presorted, unsigned int maxSize)
{
    bool bDone = true;
    ctx_.GetRenderContext().GetContext()->CSSetShader(sortStep_, nullptr, 0);

    unsigned int numThreadGroups = 0;

    if (maxSize > presorted)
    {
        if (maxSize > presorted * 2)
            bDone = false;

        unsigned int pow2 = presorted;
        while (pow2 < maxSize)
            pow2 *= 2;
        numThreadGroups = pow2 >> 9;
    }

    unsigned int nMergeSize = presorted * 2;
    for (unsigned int nMergeSubSize = nMergeSize >> 1; nMergeSubSize > 256; nMergeSubSize = nMergeSubSize >> 1)
    {
        D3D11_MAPPED_SUBRESOURCE MappedResource;

        ctx_.GetRenderContext().GetContext()->Map(dispatchInfoBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
        auto sc = (SortConstants*)MappedResource.pData;
        sc->x = nMergeSubSize;
        if (nMergeSubSize == nMergeSize >> 1)
        {
            sc->y = (2 * nMergeSubSize - 1);
            sc->z = -1;
        }
        else
        {
            sc->y = nMergeSubSize;
            sc->z = 1;
        }
        sc->w = 0;
        ctx_.GetRenderContext().GetContext()->Unmap(dispatchInfoBuffer_, 0);

        ctx_.GetRenderContext().GetContext()->Dispatch(numThreadGroups, 1, 1);
    }

    ctx_.GetRenderContext().GetContext()->CSSetShader(sortInner512_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->Dispatch(numThreadGroups, 1, 1);

    return bDone;
}
