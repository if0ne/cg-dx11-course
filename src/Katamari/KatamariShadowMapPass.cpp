#include "KatamariShadowMapPass.h"

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

KatamariShadowMapPass::KatamariShadowMapPass(std::string&& shaderPath, std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr, KatamariGame& game) :
	game_(game),
	RenderPass(std::move(shaderPath), std::move(vertexAttr))
{
}

void KatamariShadowMapPass::Initialize() {
	RenderPass::Initialize();

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = 2048;
	textureDesc.Height = 2048;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.SampleDesc.Count = 1;

	ctx_.GetRenderContext().GetDevice()->CreateTexture2D(&textureDesc, NULL, &texture_);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	ctx_.GetRenderContext().GetDevice()->CreateDepthStencilView(texture_, &depthStencilViewDesc, &dsv_);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	ctx_.GetRenderContext().GetDevice()->CreateShaderResourceView(texture_, &shaderResourceViewDesc, &srv_);

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

	viewProj_ = CreateBuffer(sizeof(Matrix));
	modelBuffer_ = CreateBuffer(sizeof(Matrix));
}

void KatamariShadowMapPass::Execute() {
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };

	ctx_.GetRenderContext().GetContext()->OMSetDepthStencilState(depthStencilState_, 1);

	ctx_.GetRenderContext().GetContext()->RSSetState(rastState_);
	ctx_.GetRenderContext().GetContext()->IASetInputLayout(layout_);
	ctx_.GetRenderContext().GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx_.GetRenderContext().GetContext()->VSSetShader(vertexShader_, nullptr, 0);
	ctx_.GetRenderContext().GetContext()->PSSetShader(pixelShader_, nullptr, 0);

	auto dir = game_.directionalLight_->Direction();
	auto data = Matrix::CreateLookAt(Vector3(0.0, 10.0, 0.0), Vector3(0.0, 10.0, 0.0) + dir, Vector3::Up) * Matrix::CreateOrthographic(100, 100, 0.0001, 1000);

	ctx_.GetRenderContext().GetContext()->OMSetRenderTargets(0, nullptr, dsv_);
	ctx_.GetRenderContext().GetContext()->ClearDepthStencilView(dsv_, D3D11_CLEAR_DEPTH, 1.0, 0);

	ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(0, 1, &viewProj_);
	ctx_.GetRenderContext().GetContext()->VSSetConstantBuffers(1, 1, &modelBuffer_);

	UpdateBuffer(viewProj_, &data, sizeof(Matrix));

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

void KatamariShadowMapPass::DestroyResources() {
	RenderPass::DestroyResources();

	texture_->Release();
	srv_->Release();

	dsv_->Release();

	depthStencilState_->Release();
	sampler_->Release();
}
