#pragma once
#include "GameComponent.h"
#include "Camera.h"

#include <SimpleMath.h>
#include <d3d.h>
#include <d3d11.h>

class Game;

struct EmitterProperties
{
    alignas(16) DirectX::SimpleMath::Vector3 position;
    alignas(16) DirectX::SimpleMath::Vector3 velocity;
    alignas(16) DirectX::SimpleMath::Vector3 positionVar;
    int maxNumToEmit;
    float particleLifeSpan;
    float particleInitRadius;
    float velocityPosVar;
    float minVelocity;
    float maxVelocity;
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

    ID3D11VertexShader* vertexShader_;
    ID3D11PixelShader* pixelShader_;

    ID3DBlob* vertexBlob_;
    ID3DBlob* pixelBlob_;

    ID3D11Buffer* particlePool;
    ID3D11UnorderedAccessView* particlePoolUAV;
    ID3D11ShaderResourceView* particlePoolSRV;

    ID3D11Buffer* deadList;
    ID3D11UnorderedAccessView* deadListUAV;

    ID3D11Buffer* aliveList;
    ID3D11UnorderedAccessView* aliveListUAV;
    ID3D11ShaderResourceView* aliveListSRV;

    ID3D11Buffer* indirectDraw;
    ID3D11UnorderedAccessView* indirectDrawUAV;

    ID3D11Buffer* viewSpacePosnR;
    ID3D11UnorderedAccessView* viewSpacePosnRUAV;
    ID3D11ShaderResourceView* viewSpacePosnRSRV;

    ID3D11Buffer* indexBuffer;

    ID3D11BlendState* blendState_;

    EmitterProperties emitterProps_;
    ID3D11Buffer* emitterBuffer_;

    ID3D11Buffer* deadCounterBuffer_;
    ID3D11Buffer* aliveCounterBuffer_;
    ID3D11Buffer* frameTimeBuffer_;
    ID3D11Buffer* viewProjBuffer_;

    ID3D11Resource* randomTexture_;
    ID3D11ShaderResourceView* randomSrv_;

    ID3D11Resource* albedoTexture_;
    ID3D11ShaderResourceView* albedoSrv_;

    ID3D11SamplerState* sampler_;

    ID3D11RasterizerState* rastState_;

    Camera* camera_;

    void Sort();
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

