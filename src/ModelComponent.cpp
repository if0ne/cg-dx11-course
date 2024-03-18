#include "ModelComponent.h"

#include "Game.h"
#include "RenderContext.h"

using namespace DirectX::SimpleMath;

void ModelComponent::ProcessNode(const aiNode* node, const aiScene* scene) {
    for (int i = 0; i < node->mNumMeshes; i++) {
        ModelComponent* cmp = new ModelComponent(path_);

        auto mesh = scene->mMeshes[node->mMeshes[i]];

        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex v {
                Vector3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
                Vector2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
            };

            cmp->vertices_.push_back(std::move(v));
        }

        for (int i = 0; i < mesh->mNumFaces; i++)
        {
            const aiFace face = mesh->mFaces[i];

            for (int j = 0; j < face.mNumIndices; j++) {
                cmp->indices_.push_back(face.mIndices[j]);
            } 
        }

        children_.push_back(cmp);
    }

    for (int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
}

void ModelComponent::Initialize() {
    if (vertices_.size() == 0) {
        for (auto& child : children_) {
            child->Initialize();
            return;
        }
    }

    D3D11_BUFFER_DESC vertexBufDesc = {};
    vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufDesc.CPUAccessFlags = 0;
    vertexBufDesc.MiscFlags = 0;
    vertexBufDesc.StructureByteStride = 0;
    vertexBufDesc.ByteWidth = sizeof(Vertex) * vertices_.size();

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
    indexBufDesc.ByteWidth = sizeof(int) * 36;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices_.data();
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    ctx_.GetRenderContext().GetDevice()->CreateBuffer(&indexBufDesc, &indexData, &ib_);
}

void ModelComponent::Update(float deltaTime) {
}

void ModelComponent::Draw() {
    UINT strides[] = { sizeof(Vector3) * 2 };
    UINT offsets[] = { 0 };

    ctx_.GetRenderContext().GetContext()->IASetIndexBuffer(ib_, DXGI_FORMAT_R32_UINT, 0);
    ctx_.GetRenderContext().GetContext()->IASetVertexBuffers(0, 1, &vb_, strides, offsets);

    ctx_.GetRenderContext().GetContext()->DrawIndexed(36, 0, 0);
}

void ModelComponent::Reload() {
}

void ModelComponent::DestroyResources() {
    vb_->Release();
    ib_->Release();
}
