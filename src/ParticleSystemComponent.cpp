#include "ParticleSystemComponent.h"
#include "Game.h"
#include "RenderContext.h"
#include <d3dcompiler.h>
#include <iostream>
#include <WICTextureLoader.h>
#include <random>

int align(int value, int alignment)
{
    return (value + (alignment - 1)) & ~(alignment - 1);
}

void ParticleSystemComponent::Initialize()
{
    emitterProps_ = {
        {0.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {1.0, 1.0, 1.0},
        10,
        2,
        1,
        10,
        0.1,
        2.0
    };

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
    }

    {
        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = MAX_PARTICLE_COUNT * sizeof(Particle);
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride = sizeof(Particle);

        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&bufferDesc, nullptr, &particlePool);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.Format = DXGI_FORMAT_UNKNOWN;
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uav_desc.Buffer.FirstElement = 0;
        uav_desc.Buffer.Flags = 0;
        uav_desc.Buffer.NumElements = MAX_PARTICLE_COUNT;
        ctx_.GetRenderContext().GetDevice()->CreateUnorderedAccessView(particlePool, &uav_desc, &particlePoolUAV);

        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
        srv_desc.Format = DXGI_FORMAT_UNKNOWN;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srv_desc.Buffer.ElementOffset = 0;
        srv_desc.Buffer.NumElements = MAX_PARTICLE_COUNT;

        ctx_.GetRenderContext().GetDevice()->CreateShaderResourceView(particlePool, &srv_desc, &particlePoolSRV);
    }

    {
        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = MAX_PARTICLE_COUNT * sizeof(uint32_t);
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride = sizeof(uint32_t);
        uint32_t indices[MAX_PARTICLE_COUNT];
        for (uint32_t i = 0; i < MAX_PARTICLE_COUNT; ++i)
        {
            indices[i] = i;
        }
        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = indices;
        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&bufferDesc, &initData, &deadList);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.Format = DXGI_FORMAT_UNKNOWN;
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uav_desc.Buffer.FirstElement = 0;
        uav_desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
        uav_desc.Buffer.NumElements = MAX_PARTICLE_COUNT;
        ctx_.GetRenderContext().GetDevice()->CreateUnorderedAccessView(deadList, &uav_desc, &deadListUAV);

        UINT initCounts[1] = { MAX_PARTICLE_COUNT };
        ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(1, 1, &deadListUAV, initCounts);
        ID3D11UnorderedAccessView* nul[1] = { nullptr };
        ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(1, 1, nul, 0);
    }

    {
        struct IndexBufferElement
        {
            float distance; 
            float index;
        };

        D3D11_BUFFER_DESC buff_desc{};
        buff_desc.ByteWidth = sizeof(IndexBufferElement) * MAX_PARTICLE_COUNT;
        buff_desc.Usage = D3D11_USAGE_DEFAULT;
        buff_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        buff_desc.CPUAccessFlags = 0;
        buff_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        buff_desc.StructureByteStride = sizeof(IndexBufferElement);

        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&buff_desc, nullptr, &aliveList);

        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
        srv_desc.Format = DXGI_FORMAT_UNKNOWN;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srv_desc.Buffer.ElementOffset = 0;
        srv_desc.Buffer.NumElements = MAX_PARTICLE_COUNT;

        ctx_.GetRenderContext().GetDevice()->CreateShaderResourceView(aliveList, &srv_desc, &aliveListSRV);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uav_desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
        uav_desc.Buffer.NumElements = MAX_PARTICLE_COUNT;
        uav_desc.Format = DXGI_FORMAT_UNKNOWN;
        ctx_.GetRenderContext().GetDevice()->CreateUnorderedAccessView(aliveList, &uav_desc, &aliveListUAV);
    }

    {
        D3D11_BUFFER_DESC buff_desc{};
        buff_desc.Usage = D3D11_USAGE_DEFAULT;
        buff_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
        buff_desc.ByteWidth = 5 * sizeof(UINT);
        buff_desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&buff_desc, nullptr, &indirectDraw);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.Format = DXGI_FORMAT_R32_UINT;
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uav_desc.Buffer.FirstElement = 0;
        uav_desc.Buffer.NumElements = 5;
        uav_desc.Buffer.Flags = 0;
        ctx_.GetRenderContext().GetDevice()->CreateUnorderedAccessView(indirectDraw, &uav_desc, &indirectDrawUAV);
    }

    {
        D3D11_BUFFER_DESC buff_desc{};
        buff_desc.ByteWidth = sizeof(DirectX::SimpleMath::Vector4) * MAX_PARTICLE_COUNT;
        buff_desc.Usage = D3D11_USAGE_DEFAULT;
        buff_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        buff_desc.CPUAccessFlags = 0;
        buff_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        buff_desc.StructureByteStride = sizeof(DirectX::SimpleMath::Vector4);

        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&buff_desc, nullptr, &viewSpacePosnR);

        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
        srv_desc.Format = DXGI_FORMAT_UNKNOWN;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srv_desc.Buffer.ElementOffset = 0;
        srv_desc.Buffer.NumElements = MAX_PARTICLE_COUNT;

        ctx_.GetRenderContext().GetDevice()->CreateShaderResourceView(viewSpacePosnR, &srv_desc, &viewSpacePosnRSRV);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.Buffer.NumElements = MAX_PARTICLE_COUNT;
        uav_desc.Buffer.Flags = 0;
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uav_desc.Format = DXGI_FORMAT_UNKNOWN;
        ctx_.GetRenderContext().GetDevice()->CreateUnorderedAccessView(viewSpacePosnR, &uav_desc, &viewSpacePosnRUAV);
    }

    {
        D3D11_BUFFER_DESC buff_desc{};
        buff_desc.ByteWidth = MAX_PARTICLE_COUNT * 6 * sizeof(UINT);
        buff_desc.Usage = D3D11_USAGE_IMMUTABLE;
        buff_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        buff_desc.CPUAccessFlags = 0;
        buff_desc.MiscFlags = 0;
        D3D11_SUBRESOURCE_DATA data;

        std::vector<UINT> indices(MAX_PARTICLE_COUNT * 6);
        data.pSysMem = indices.data();
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;

        UINT base = 0;
        for (int i = 0; i < MAX_PARTICLE_COUNT * 6; i += 6)
        {
            indices[i] = base + 0;
            indices[i + 1] = base + 1;
            indices[i + 2] = base + 2;

            indices[i + 3] = base + 2;
            indices[i + 4] = base + 1;
            indices[i + 5] = base + 3;

            base += 4;
        }

        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&buff_desc, &data, &indexBuffer);
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
        constBufDesc.ByteWidth = sizeof(SBCounterS);

        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&constBufDesc, nullptr, &deadCounterBuffer_);
    }

    {
        D3D11_BUFFER_DESC constBufDesc = {};
        constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
        constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constBufDesc.MiscFlags = 0;
        constBufDesc.StructureByteStride = 0;
        constBufDesc.ByteWidth = sizeof(SBCounterS);

        ctx_.GetRenderContext().GetDevice()->CreateBuffer(&constBufDesc, nullptr, &aliveCounterBuffer_);
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
            D3D11_CULL_BACK,
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
    ctx_.GetRenderContext().GetContext()->CopyStructureCount(aliveCounterBuffer_, 0, aliveListUAV);

    Sort();

    ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);

    ID3D11ShaderResourceView* vs_srv[] = {
        particlePoolSRV, viewSpacePosnRSRV, aliveListSRV
    };

    ID3D11Buffer* vb = nullptr;
    UINT stride = 0;
    UINT offset = 0;

    ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(3, 1, &aliveCounterBuffer_);
    ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    ctx_.GetRenderContext().GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ctx_.GetRenderContext().GetContext()->VSSetShaderResources(0, ARRAYSIZE(vs_srv), vs_srv);

    ctx_.GetRenderContext().GetContext()->PSSetShaderResources(0, 1, &albedoSrv_);
    ctx_.GetRenderContext().GetContext()->PSSetSamplers(0, 1, &sampler_);
    ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
    ctx_.GetRenderContext().GetContext()->OMSetBlendState(blendState_, nullptr, 0xffffffff);
    ctx_.GetRenderContext().GetContext()->DrawIndexedInstancedIndirect(indirectDraw, 0);

    ZeroMemory(vs_srv, sizeof(vs_srv));
    ctx_.GetRenderContext().GetContext()->VSSetShaderResources(0, ARRAYSIZE(vs_srv), vs_srv);

    ctx_.GetRenderContext().GetContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void ParticleSystemComponent::Reload()
{
}

void ParticleSystemComponent::DestroyResources()
{
}

void ParticleSystemComponent::Sort()
{
}

void ParticleSystemComponent::Emit()
{
    ID3D11UnorderedAccessView* uavs[] = { particlePoolUAV, deadListUAV };
    UINT initialCounts[] = { (UINT)-1, (UINT)-1 };
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(0, std::size(uavs), uavs, initialCounts);

    ID3D11Buffer* buffers[] = { frameTimeBuffer_, emitterBuffer_, deadCounterBuffer_, viewProjBuffer_ };
    auto time = DirectX::SimpleMath::Vector4(ctx_.GetDeltaTime(), 0.0, 0.0, 0.0);
    DirectX::SimpleMath::Matrix matrices[2] = {camera_->View(), camera_->Projection()};

    UpdateBuffer(frameTimeBuffer_, &time, sizeof(DirectX::SimpleMath::Vector4));
    UpdateBuffer(emitterBuffer_, &emitterProps_, sizeof(EmitterProperties));
    UpdateBuffer(viewProjBuffer_, &matrices, 2 * sizeof(DirectX::SimpleMath::Matrix));
    ctx_.GetRenderContext().GetContext()->CSSetConstantBuffers(0, 4, buffers);

    ID3D11ShaderResourceView* srvs[] = { randomSrv_ };
    ctx_.GetRenderContext().GetContext()->CSSetShaderResources(0, 1, srvs);
    ctx_.GetRenderContext().GetContext()->CSSetSamplers(0, 1, &sampler_);

    ctx_.GetRenderContext().GetContext()->CSSetShader(emitCs_, nullptr, 0);

    ctx_.GetRenderContext().GetContext()->CopyStructureCount(deadCounterBuffer_, 0, deadListUAV);

    int numThreadGroups = align(emitterProps_.maxNumToEmit, X_NUMTHREADS) / X_NUMTHREADS;
    ctx_.GetRenderContext().GetContext()->Dispatch(numThreadGroups, 1, 1);
}

void ParticleSystemComponent::Simulate()
{
    ID3D11UnorderedAccessView* uavs[] = {
                particlePoolUAV, deadListUAV, aliveListUAV, viewSpacePosnRUAV, indirectDrawUAV
    };

    UINT initialCounts[] = { (UINT)-1, (UINT)-1, 0, (UINT)-1, (UINT)-1 };

    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavs), uavs, initialCounts);

    ctx_.GetRenderContext().GetContext()->CSSetShader(simulateCs_, nullptr, 0);
    ID3D11Buffer* buffers[] = { frameTimeBuffer_, emitterBuffer_, deadCounterBuffer_, viewProjBuffer_ };
    ctx_.GetRenderContext().GetContext()->CSSetConstantBuffers(0, 4, buffers);
    ctx_.GetRenderContext().GetContext()->Dispatch(align(MAX_PARTICLE_COUNT, 256) / 256, 1, 1);

    ZeroMemory(uavs, sizeof(uavs));
    ctx_.GetRenderContext().GetContext()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavs), uavs, nullptr);
}

void ParticleSystemComponent::UpdateBuffer(ID3D11Buffer* buffer, void* data, size_t size) {
    D3D11_MAPPED_SUBRESOURCE res = {};
    ctx_.GetRenderContext().GetContext()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, data, size);
    ctx_.GetRenderContext().GetContext()->Unmap(buffer, 0);
}