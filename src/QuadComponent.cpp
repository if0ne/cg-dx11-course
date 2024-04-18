#include "QuadComponent.h"

#include <math.h>

#include "Game.h"
#include "RenderContext.h"

using namespace DirectX::SimpleMath;

void QuadComponent::Initialize() {
    vertices_.push_back(Vector3(-1.0, -1.0, 0.0)); vertices_.push_back(Vector3::Zero);
    vertices_.push_back(Vector3(-1.0, 1.0, 0.0)); vertices_.push_back(Vector3::Zero);
    vertices_.push_back(Vector3(1.0, 1.0, 0.0)); vertices_.push_back(Vector3::Zero);
    vertices_.push_back(Vector3(1.0, -1.0, 0.0)); vertices_.push_back(Vector3::Zero);

    indices_ = { 0, 1, 2, 2, 3, 0 };

    D3D11_BUFFER_DESC vertexBufDesc = {};
    vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufDesc.CPUAccessFlags = 0;
    vertexBufDesc.MiscFlags = 0;
    vertexBufDesc.StructureByteStride = 0;
    vertexBufDesc.ByteWidth = sizeof(Vector3) * vertices_.size();

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
}

void QuadComponent::Update(float deltaTime) {
}

void QuadComponent::Draw() {
    UINT strides[] = { sizeof(Vector3) * 2 };
    UINT offsets[] = { 0 };

    ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(ib_, DXGI_FORMAT_R32_UINT, 0);
    ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &vb_, strides, offsets);

    ctx_.GetRenderContext().GetContext()->DrawIndexed(indices_.size(), 0, 0);
}

void QuadComponent::Reload() {
}

void QuadComponent::DestroyResources() {
    vb_->Release();
    ib_->Release();
}