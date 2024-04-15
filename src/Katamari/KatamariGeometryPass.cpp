#include "KatamariGeometryPass.h"

#include "../Game.h"
#include "../Camera.h"
#include "../RenderContext.h"
#include "../Window.h"
#include "../ModelComponent.h"
#include "../MeshComponent.h"

#include "KatamariGame.h"
#include "PlayerComponent.h"
#include "StickyObjectComponent.h"

#include <SimpleMath.h>
#include <array>

using namespace DirectX::SimpleMath;

KatamariGeometryPass::KatamariGeometryPass(
    std::string&& shaderPath, 
    std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr, 
    CD3D11_RASTERIZER_DESC rastState, 
    KatamariGame& game
) :
    game_(game),
    RenderPass(std::move(shaderPath), std::move(vertexAttr), rastState)
{
}

KatamariGeometryPass::~KatamariGeometryPass() {
}

void KatamariGeometryPass::Initialize() {
    RenderPass::Initialize();

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = ctx_.GetWindow().GetWidth();
	textureDesc.Height = ctx_.GetWindow().GetHeight();
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	for (UINT i = 0; i < 4; i++)
		ctx_.GetRenderContext().GetDevice()->CreateTexture2D(&textureDesc, NULL, &textures_[i]);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	for (UINT i = 0; i < 4; i++)
		ctx_.GetRenderContext().GetDevice()->CreateRenderTargetView(textures_[i], &renderTargetViewDesc, &rtvs_[i]);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	for (UINT i = 0; i < 4; i++)
		ctx_.GetRenderContext().GetDevice()->CreateShaderResourceView(textures_[i], &shaderResourceViewDesc, &srvs_[i]);

	textureDesc = {};
	textureDesc.Width = ctx_.GetWindow().GetWidth();
	textureDesc.Height = ctx_.GetWindow().GetHeight();
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ctx_.GetRenderContext().GetDevice()->CreateTexture2D(&textureDesc, NULL, &depthTexture_);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	ctx_.GetRenderContext().GetDevice()->CreateDepthStencilView(depthTexture_, &depthStencilViewDesc, &dsv_);

	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc{};

	depthStencilStateDesc.DepthEnable = TRUE;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilStateDesc.StencilEnable = FALSE;

	ctx_.GetRenderContext().GetDevice()->CreateDepthStencilState(&depthStencilStateDesc, &depthStencilState_);


	wvpBuffer_ = CreateBuffer(sizeof(DirectX::SimpleMath::Matrix));
	modelBuffer_ = CreateBuffer(sizeof(DirectX::SimpleMath::Matrix));
	materialBuffer_ = CreateBuffer(sizeof(Material));
}

void KatamariGeometryPass::Execute() {
    ctx_.SetViewport(0, 0, ctx_.GetWindow().GetWidth(), ctx_.GetWindow().GetHeight());

    ctx_.GetRenderContext().GetContext()->OMSetRenderTargets(4, rtvs_, dsv_);

    std::array<float, 4> color = { 0.0f, 0.0f, 0.0f, 1.0f };
    ctx_.GetRenderContext().GetContext()->ClearRenderTargetView(rtvs_[0], color.data());
    color = { 0.0f, 0.0f, -1.0f, 1.0f };
    ctx_.GetRenderContext().GetContext()->ClearRenderTargetView(rtvs_[1], color.data());
    color = { 0.0f, 0.0f, 0.0f, 1.0f };
    ctx_.GetRenderContext().GetContext()->ClearRenderTargetView(rtvs_[2], color.data());
    ctx_.GetRenderContext().GetContext()->ClearRenderTargetView(rtvs_[3], color.data());
    ctx_.GetRenderContext().GetContext()->ClearDepthStencilView(dsv_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);

    ctx_.GetRenderContext().GetContext()->OMSetDepthStencilState(depthStencilState_, 0);

    ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
    ctx_.GetRenderContext().GetContext()->IASetInputLayout(layout_);
    ctx_.GetRenderContext().GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
    ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);

    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(0, 1, &wvpBuffer_);
    ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(1, 1, &modelBuffer_);

    ctx_.GetRenderContext().GetContext()->PSSetConstantBuffers(2, 1, &materialBuffer_);

    auto matrix = game_.camera_->CameraMatrix();
    UpdateBuffer(wvpBuffer_, &matrix, sizeof(Matrix));

    auto renderData = game_.player_->GetRenderData();

    UINT strides[] = { sizeof(Vertex) };
    UINT offsets[] = { 0 };

    UpdateBuffer(modelBuffer_, &renderData.transform, sizeof(Matrix));
    UpdateBuffer(materialBuffer_, &renderData.material, sizeof(Material));

    for (auto& mesh : renderData.meshData) {
        ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(mesh.ib, DXGI_FORMAT_R32_UINT, 0);
        ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &mesh.vb, strides, offsets);
        ctx_.GetRenderContext().GetContext()->PSSetShaderResources(0, 1, &mesh.texture);
        ctx_.GetRenderContext().GetContext()->PSSetShaderResources(1, 1, &mesh.normal);
        ctx_.GetRenderContext().GetContext()->PSSetSamplers(0, 1, &mesh.sampler);

        ctx_.GetRenderContext().GetContext()->DrawIndexed(mesh.indexCount, 0, 0);
    }


    for (auto& object : game_.objects_) {
        renderData = object->GetRenderData();
        UpdateBuffer(modelBuffer_, &renderData.transform, sizeof(Matrix));
        UpdateBuffer(materialBuffer_, &renderData.material, sizeof(Material));

        for (auto& mesh : renderData.meshData) {
            ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(mesh.ib, DXGI_FORMAT_R32_UINT, 0);
            ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &mesh.vb, strides, offsets);
            ctx_.GetRenderContext().GetContext()->PSSetShaderResources(0, 1, &mesh.texture);
            ctx_.GetRenderContext().GetContext()->PSSetShaderResources(1, 1, &mesh.normal);
            ctx_.GetRenderContext().GetContext()->PSSetSamplers(0, 1, &mesh.sampler);

            ctx_.GetRenderContext().GetContext()->DrawIndexed(mesh.indexCount, 0, 0);
        }
    }

    auto groundMatrix = Matrix::CreateTranslation(Vector3(0, -4.0, 0));
    UpdateBuffer(modelBuffer_, &groundMatrix, sizeof(Matrix));

    auto mat = Material{
        Vector4(1.0, 1.0, 1.0, 1.0),
        1.0,
        0.01,
        1.0,
        0.0
    };
    UpdateBuffer(materialBuffer_, &mat, sizeof(Material));
    auto mesh = game_.ground_->GetMeshRenderData();

    for (auto& mesh : mesh) {
        ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(mesh.ib, DXGI_FORMAT_R32_UINT, 0);
        ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &mesh.vb, strides, offsets);
        ctx_.GetRenderContext().GetContext()->PSSetShaderResources(0, 1, &mesh.texture);
        ctx_.GetRenderContext().GetContext()->PSSetShaderResources(1, 1, &mesh.normal);
        ctx_.GetRenderContext().GetContext()->PSSetSamplers(0, 1, &mesh.sampler);

        ctx_.GetRenderContext().GetContext()->DrawIndexed(mesh.indexCount, 0, 0);
    }
}

void KatamariGeometryPass::DestroyResources() {
    RenderPass::DestroyResources();

	depthTexture_->Release();
	dsv_->Release();
	depthStencilState_->Release();

	for (UINT i = 0; i < 4; i++) {
		textures_[i]->Release();
		rtvs_[i]->Release();
		srvs_[i]->Release();
	}
}
