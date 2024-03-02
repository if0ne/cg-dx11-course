#pragma once
#include "../GameComponent.h"

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <SimpleMath.h>

class Camera;
class CameraController;

class CubeComponent;

class SunSystemGame : public GameComponent
{
private:
    DirectX::SimpleMath::Vector3 center_;
    Camera* camera_;
    CameraController* cameraController_;

    CubeComponent* cube_;

    ID3D11InputLayout* layout_;
    ID3D11VertexShader* vertexShader_;
    ID3D11PixelShader* pixelShader_;

    ID3DBlob* vertexBC_;
    ID3DBlob* pixelBC_;

    ID3D11RasterizerState* rastState_;

    ID3D11Buffer* constBuffer_;

public:
    SunSystemGame();
    ~SunSystemGame();

    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void Reload() override;
    virtual void DestroyResources() override;
};

