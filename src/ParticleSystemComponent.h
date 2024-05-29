#pragma once
#include "GameComponent.h"
#include "Camera.h"

#include <SimpleMath.h>
#include <d3d.h>
#include <d3d11.h>

class Game;

struct alignas(16) EmitterProperties
{
    DirectX::SimpleMath::Vector3 position;
    int maxNumToEmit;
    float particleLifeSpan;
};

struct Particle
{
    DirectX::SimpleMath::Vector3 positon;
    DirectX::SimpleMath::Vector3 velocity;
    DirectX::SimpleMath::Vector3 color;
    float rotation;
    float age;
    float radius;
    float maxLife;
    float distToEye;
};

struct alignas(16) NewParticle {
    DirectX::SimpleMath::Vector3 positon;
    DirectX::SimpleMath::Vector3 velocity;
    float age;
};

struct alignas(16) SBCounterS
{
    alignas(16) UINT deadCounter;
};

class ParticleSystemComponent : GameComponent
{
private:
    ID3DBlob* emitBlobCs_;
    ID3D11ComputeShader* emitCs_;

    ID3DBlob* simulateBlobCs_;
    ID3D11ComputeShader* simulateCs_;

    ID3DBlob* resetBlboCs_;
    ID3D11ComputeShader* resetCs_;

    ID3D11VertexShader* vertexShader_;
    ID3D11PixelShader* pixelShader_;
    ID3D11GeometryShader* geometryShader_;

    ID3DBlob* vertexBlob_;
    ID3DBlob* pixelBlob_;
    ID3DBlob* geometryBlob_;

    ID3D11BlendState* blendState_;

    EmitterProperties emitterProps_;
    ID3D11Buffer* emitterBuffer_;

    ID3D11Buffer* frameTimeBuffer_;
    ID3D11Buffer* viewProjBuffer_;

    ID3D11Resource* randomTexture_;
    ID3D11ShaderResourceView* randomSrv_;

    ID3D11Resource* albedoTexture_;
    ID3D11ShaderResourceView* albedoSrv_;

    ID3D11SamplerState* sampler_;

    ID3D11RasterizerState* rastState_;
    ID3D11DepthStencilState* depthState_;

    Camera* camera_;

    ID3D11Buffer* dispatchInfoBuffer_;
    ID3D11ComputeShader* sortStep_;
    ID3D11ComputeShader* sort512_;
    ID3D11ComputeShader* sortInner512_; 
    ID3D11ComputeShader* sortInitArgs_; 

    ID3D11Buffer* particleBuffer_;
    ID3D11ShaderResourceView* particleBufferSrv_;
    ID3D11UnorderedAccessView* particleBufferUav_;

    ID3D11Buffer* deadBuffer_;
    ID3D11UnorderedAccessView* deadBufferUav_;

    ID3D11Buffer* aliveBuffer_[2];
    ID3D11ShaderResourceView* aliveBufferSrv_[2];
    ID3D11UnorderedAccessView* aliveBufferUav_[2];

    ID3D11UnorderedAccessView* aliveBufferSortingUav_[2];

    ID3D11Buffer* aliveCounterBuffer_;
    ID3D11Buffer* deadCounterBuffer_;

    ID3D11Buffer* indirectBuffer_[2];
    ID3D11UnorderedAccessView* indirectBufferUav_[2];

    ID3D11Buffer* indirectDrawBuffer_;
    ID3D11UnorderedAccessView* indirectDrawBufferUav_;

    int currentAliveBuffer_;

    void Sort();
    bool SortInitial(unsigned int maxSize);
    bool SortIncremental(unsigned int presorted, unsigned int maxSize);
    void Emit();
    void Simulate();
public:
    static constexpr int MAX_PARTICLE_COUNT = 100000;
    static constexpr int X_NUMTHREADS = 1024;

    ParticleSystemComponent(Camera* camera) : camera_(camera), GameComponent() {}

    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void Reload();
    virtual void DestroyResources();
    void UpdateBuffer(ID3D11Buffer* buffer, void* data, size_t size);
};

