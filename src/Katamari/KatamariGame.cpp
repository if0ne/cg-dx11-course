#include "KatamariGame.h"

#include <string>
#include <vector>
#include <SimpleMath.h>

#include "../AssetLoader.h"
#include "../Game.h"
#include "../InputDevice.h"
#include "../Camera.h"
#include "../OrbitCameraController.h"
#include "../SphereComponent.h"
#include "../DirectionalLightComponent.h"
#include "../AmbientLightComponent.h"
#include "../PointLightComponent.h"
#include "../ModelComponent.h"

#include "PlayerComponent.h"
#include "StickyObjectComponent.h"
#include "KatamariRenderPass.h"

using namespace DirectX::SimpleMath;

KatamariGame::KatamariGame() : GameComponent() {
    camera_ = new Camera();
    player_ = new PlayerComponent(*camera_, *this);

    auto path = std::string("/bball.fbx");
    std::vector<std::tuple<std::string, float, Material>> models;
    models.push_back(std::make_tuple(std::string("/rock.fbx"), 1.0, Material{
        Vector4(1.0, 1.0, 1.0, 1.0),
        1.0,
        1.0,
        1.0,
        0.0
    }));

    models.push_back(std::make_tuple(std::string("/pump.fbx"), 1.0, Material{
        Vector4(1.0, 1.0, 1.0, 1.0),
        1.0,
        1.0,
        1.0,
        0.0,
    }));

    models.push_back(std::make_tuple(std::string("/wheel.fbx"), 0.005, Material{
        Vector4(1.0, 1.0, 1.0, 1.0),
        1.0,
        1.0,
        1.0,
        0.0
    }));

    for (int i = 0; i < 10; i++) {
        int random = rand() % models.size();
        int x = rand() % 40 - 20;
        int z = rand() % 40 - 20;
        objects_.push_back(new StickyObjectComponent(
            std::get<0>(models[random]), 
            std::get<1>(models[random]), 
            Vector3(x, 0, z), 
            std::get<2>(models[random]), 
            *this
        ));
    }

    directionalLight_ = new DirectionalLightComponent(Vector3(0.25, -1.0, 0.0), Vector3(1.0, 1.0, 1.0), 1.0);
    ambientLight_ = new AmbientLightComponent(Vector3(0.04, 0.14, 0.72), 0.23);
    pointLight_ = new PointLightComponent(Vector3(0.0, 4.0, 0.0), 16.0, Vector3(0.94, 0.14, 0.0), 5.0);

    auto shaderPath = std::string("./shaders/KatamariLightShadow.hlsl");
    std::vector<std::pair<const char*, DXGI_FORMAT>> vertexAttr{
        std::make_pair("POSITION", DXGI_FORMAT_R32G32B32_FLOAT),
        std::make_pair("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT),
        std::make_pair("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT),
        std::make_pair("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
    };

    mainPass_ = new KatamariRenderPass(std::move(shaderPath), std::move(vertexAttr), *this);
}

KatamariGame::~KatamariGame() {
    delete player_;
    delete camera_;
    delete directionalLight_;
    delete ambientLight_;

    for (auto& object : objects_) {
        delete object;
    }
}

void KatamariGame::Initialize() {
    ctx_.GetWindow().HideWindowCursor();

    mainPass_->Initialize();

    player_->Initialize();

    for (auto& object : objects_) {
        object->Initialize();
    }

    auto path = std::string("/Floor_plate.fbx");
    ground_ = ctx_.GetAssetLoader().LoadModel(path);
}

void KatamariGame::Update(float deltaTime) {
    if (ctx_.GetInputDevice().IsKeyDown(Keys::Escape)) {
        ctx_.Exit();
    }

    if (ctx_.GetInputDevice().IsKeyDown(Keys::LeftShift)) {
        ctx_.GetWindow().ShowWindowCursor();
    } else {
        ctx_.GetWindow().HideWindowCursor();
        ctx_.GetWindow().LockWindowCursor();
    }

    player_->Update(deltaTime);

    for (auto& object : objects_) {
        object->Update(deltaTime);
    }
}

void KatamariGame::Draw() {
    mainPass_->Execute();
}

void KatamariGame::Reload() {
    player_->Reload();
    for (auto& object : objects_) {
        object->Reload();
    }
}

void KatamariGame::DestroyResources() {
    mainPass_->DestroyResources();
    player_->DestroyResources();
    for (auto& object : objects_) {
        object->DestroyResources();
    }
}