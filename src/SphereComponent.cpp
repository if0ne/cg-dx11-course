#define _USE_MATH_DEFINES

#include "SphereComponent.h"

#include <math.h>

#include "Game.h"
#include "RenderContext.h"

using namespace DirectX::SimpleMath;

void SphereComponent::Initialize() {
    float phiInc = M_PI / numSegments_;
    float thetaInc = 2.0 * M_PI / numSegments_;

    for (int phi = 0; phi <= numSegments_; phi++) {
        float phiAngle = phi * phiInc;
        float sinPhi = sinf(phiAngle);
        float cosPhi = cosf(phiAngle);

        for (int theta = 0; theta <= numSegments_; theta++)
        {
            float thetaAngle = theta * thetaInc;
            float sinTheta = sinf(thetaAngle);
            float cosTheta = cosf(thetaAngle);

            Vector3 position = Vector3(cosTheta * sinPhi, cosPhi, sinTheta * sinPhi);

            vertices_.push_back(position);

            if (phi % 2 == 0) {
                vertices_.push_back(firstColor_);
            } else {
                vertices_.push_back(secondColor_);
            }
        }
    }

    for (int phi = 0; phi < numSegments_; phi++)
    {
        for (int theta = 0; theta < numSegments_; theta++)
        {
            int topLeft = phi * (numSegments_ + 1) + theta;
            int topRight = topLeft + 1;
            int bottomLeft = (phi + 1) * (numSegments_ + 1) + theta;
            int bottomRight = bottomLeft + 1;

            indices_.push_back(topLeft);
            indices_.push_back(topRight);
            indices_.push_back(bottomLeft);

            indices_.push_back(bottomLeft);
            indices_.push_back(topRight);
            indices_.push_back(bottomRight);
        }
    }

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

void SphereComponent::Update(float deltaTime) {
}

void SphereComponent::Draw() {
    UINT strides[] = { sizeof(Vector3) * 2 };
    UINT offsets[] = { 0 };

    ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(ib_, DXGI_FORMAT_R32_UINT, 0);
    ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &vb_, strides, offsets);

    ctx_.GetRenderContext().GetContext()->DrawIndexed(indices_.size(), 0, 0);
}

void SphereComponent::Reload() {
}

void SphereComponent::DestroyResources() {
    vb_->Release();
    ib_->Release();
}
