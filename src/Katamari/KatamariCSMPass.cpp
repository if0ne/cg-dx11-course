#include "KatamariCSMPass.h"

#include "../Game.h"
#include "../RenderContext.h"
#include "../DirectionalLightComponent.h"
#include "../MeshComponent.h"
#include "../ModelComponent.h"

#include "KatamariGame.h"
#include "PlayerComponent.h"
#include "StickyObjectComponent.h"

#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

KatamariCSMPass::KatamariCSMPass(
	std::string&& shaderPath, 
	std::vector<std::pair<const char*, 
	DXGI_FORMAT>>&& vertexAttr, 
	CD3D11_RASTERIZER_DESC rastState,
	KatamariGame& game
) :
    game_(game),
    RenderPass(std::move(shaderPath), std::move(vertexAttr), rastState)
{
}

void KatamariCSMPass::Initialize() {
    RenderPass::Initialize();

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = csm_.kWidth;
	textureDesc.Height = csm_.kHeight;
	textureDesc.ArraySize = csm_.kCascadeCount;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.SampleDesc.Count = 1;

	ctx_.GetRenderContext().GetDevice()->CreateTexture2D(&textureDesc, NULL, &textureArray_);

	for (int i = 0; i < csm_.kCascadeCount; i++) {
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		depthStencilViewDesc.Texture2DArray.MipSlice = 0;
		depthStencilViewDesc.Texture2DArray.FirstArraySlice = i;
		depthStencilViewDesc.Texture2DArray.ArraySize = 1;
		ctx_.GetRenderContext().GetDevice()->CreateDepthStencilView(textureArray_, &depthStencilViewDesc, &dsv_[i]);
	}
	
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2DArray.MipLevels = 1;
	shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
	shaderResourceViewDesc.Texture2DArray.ArraySize = csm_.kCascadeCount;
	ctx_.GetRenderContext().GetDevice()->CreateShaderResourceView(textureArray_, &shaderResourceViewDesc, &srv_);

	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc{};

	depthStencilStateDesc.DepthEnable = TRUE;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilStateDesc.StencilEnable = FALSE;

	ctx_.GetRenderContext().GetDevice()->CreateDepthStencilState(&depthStencilStateDesc, &depthStencilState_);

	D3D11_SAMPLER_DESC sampDesc{};
	sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	//sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	//sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ctx_.GetRenderContext().GetDevice()->CreateSamplerState(&sampDesc, &sampler_);

	cascadeBuffer_ = CreateBuffer(sizeof(CascadeData));
	modelBuffer_ = CreateBuffer(sizeof(Matrix));
}

void KatamariCSMPass::Execute() {
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	ctx_.SetViewport(0, 0, csm_.kWidth, csm_.kHeight);
	ctx_.GetRenderContext().GetContext()->OMSetDepthStencilState(depthStencilState_, 1);

	ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
	ctx_.GetRenderContext().GetContext()->IASetInputLayout(layout_);
	ctx_.GetRenderContext().GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
	ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);

	auto dir = game_.directionalLight_->Direction();
	auto camera = game_.camera_;
	auto data = csm_.RenderData(dir, *camera);


	for (int i = 0; i < csm_.kCascadeCount; i++) {
		ctx_.GetRenderContext().GetContext()->OMSetRenderTargets(0, nullptr, dsv_[i]);
		ctx_.GetRenderContext().GetContext()->ClearDepthStencilView(dsv_[i], D3D11_CLEAR_DEPTH, 1.0, 0);

		ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(0, 1, &cascadeBuffer_);
		ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(1, 1, &modelBuffer_); 

		UpdateBuffer(cascadeBuffer_, &data.viewProjMat[i], sizeof(CascadeData));

		auto renderData = game_.player_->GetRenderData();

		UINT strides[] = { sizeof(Vertex) };
		UINT offsets[] = { 0 };

		UpdateBuffer(modelBuffer_, &renderData.transform, sizeof(Matrix));

		for (auto& mesh : renderData.meshData) {
			ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(mesh.ib, DXGI_FORMAT_R32_UINT, 0);
			ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &mesh.vb, strides, offsets);

			ctx_.GetRenderContext().GetContext()->DrawIndexed(mesh.indexCount, 0, 0);
		}

		for (auto& object : game_.objects_) {
			renderData = object->GetRenderData();
			UpdateBuffer(modelBuffer_, &renderData.transform, sizeof(Matrix));

			for (auto& mesh : renderData.meshData) {
				ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(mesh.ib, DXGI_FORMAT_R32_UINT, 0);
				ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &mesh.vb, strides, offsets);

				ctx_.GetRenderContext().GetContext()->DrawIndexed(mesh.indexCount, 0, 0);
			}
		}
	}
}

void KatamariCSMPass::DestroyResources() {
    RenderPass::DestroyResources();

    textureArray_->Release();
    srv_->Release();

    for (int i = 0; i < csm_.kCascadeCount; i++) {
		dsv_[i]->Release();
    }

    depthStencilState_->Release();
    sampler_->Release();
}

CSMRenderData KatamariCSMPass::RenderData() {
	auto dir = game_.directionalLight_->Direction();
	auto camera = game_.camera_;
	auto data = csm_.RenderData(dir, *camera);

	return CSMRenderData{
		srv_,
		sampler_,
		data
	};
}
