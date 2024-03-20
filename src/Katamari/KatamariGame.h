#pragma once
#include "../GameComponent.h"

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <SimpleMath.h>

#include <vector>

class Camera;
class PlayerComponent;
class StickyObjectComponent;

class KatamariGame : public GameComponent
{
private:
    Camera* camera_;

    ID3D11InputLayout* layout_;
    ID3D11VertexShader* vertexShader_;
    ID3D11PixelShader* pixelShader_;

    ID3DBlob* vertexBC_;
    ID3DBlob* pixelBC_;

    ID3D11RasterizerState* rastState_;

    ID3D11Buffer* wvpBuffer_;
    ID3D11Buffer* modelBuffer_;

    std::vector<StickyObjectComponent*> objects_;

    PlayerComponent* player_;
public:
    KatamariGame();
    ~KatamariGame();

    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void Reload() override;
    virtual void DestroyResources() override;

    void UpdateModelBuffer(DirectX::SimpleMath::Matrix& matrix);
};
