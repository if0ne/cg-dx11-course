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
class ModelComponent;
class DirectionalLightComponent;
class AmbientLightComponent;
class PointLightComponent;
struct Material;
class KatamariRenderPass;

class KatamariGame : public GameComponent
{
private:
    Camera* camera_;

    std::vector<StickyObjectComponent*> objects_;

    PlayerComponent* player_;

    ModelComponent* ground_;

    DirectionalLightComponent* directionalLight_;
    AmbientLightComponent* ambientLight_;
    PointLightComponent* pointLight_;

    KatamariRenderPass* mainPass_;
public:
    KatamariGame();
    ~KatamariGame();

    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    virtual void Reload() override;
    virtual void DestroyResources() override;

    PlayerComponent& Player() {
        return *player_;
    }

    friend class KatamariRenderPass;
    friend class KatamariCSMPass;
    friend class KatamariShadowMapPass;
    friend class KatamariGeometryPass;
};
