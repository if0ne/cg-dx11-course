#include "CubeComponent.h"

#include "Game.h"
#include "RenderContext.h"

using namespace DirectX::SimpleMath;

void CubeComponent::Initialize() {
    Vector3 verts[16] = {
        Vector3(-1.0, -1.0, -1.0), Vector3(0.0, 0.667, 1.0),
        Vector3(-1.0, 1.0, -1.0), Vector3(0.0, 0.667, 1.0),
        Vector3(1.0, 1.0, -1.0), Vector3(0.0, 0.667, 1.0),
        Vector3(1.0, -1.0, -1.0), Vector3(0.0, 0.667, 1.0),

        Vector3(-1.0, -1.0, 1.0), Vector3(0.0, 0.667, 0.0),
        Vector3(-1.0, 1.0, 1.0), Vector3(0.0, 0.667, 0.0),
        Vector3(1.0, 1.0, 1.0), Vector3(0.0, 0.667, 0.0),
        Vector3(1.0, -1.0, 1.0), Vector3(0.0, 0.667, 0.0),
    };

    memcpy(vertices_, verts, sizeof(Vector3) * 16);

    int indices[36] =
    {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7
    };

    memcpy(indices_, indices, sizeof(int) * 36);

    D3D11_BUFFER_DESC vertexBufDesc = {};
    vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufDesc.CPUAccessFlags = 0;
    vertexBufDesc.MiscFlags = 0;
    vertexBufDesc.StructureByteStride = 0;
    vertexBufDesc.ByteWidth = sizeof(Vector3) * 16;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices_;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&vertexBufDesc, &vertexData, &vb_);

    D3D11_BUFFER_DESC indexBufDesc = {};
    indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufDesc.CPUAccessFlags = 0;
    indexBufDesc.MiscFlags = 0;
    indexBufDesc.StructureByteStride = 0;
    indexBufDesc.ByteWidth = sizeof(int) * 36;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices_;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&indexBufDesc, &indexData, &ib_);
}

void CubeComponent::Update(float deltaTime) {
}

void CubeComponent::Draw() {
    UINT strides[] = { sizeof(Vector3) * 2 };
    UINT offsets[] = { 0 };

    ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(ib_, DXGI_FORMAT_R32_UINT, 0);
    ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &vb_, strides, offsets);

    ctx_.GetRenderContext().GetContext()->DrawIndexed(36, 0, 0);
}

void CubeComponent::Reload() {
}

void CubeComponent::DestroyResources() {
    vb_->Release();
    ib_->Release();
}
