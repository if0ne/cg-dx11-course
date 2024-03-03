#include "PlanetComponent.h"

#include "../SphereComponent.h"
#include "SunSystemGame.h"

PlanetComponent::PlanetComponent(SunSystemGame& game, DirectX::SimpleMath::Vector3 position) : 
    game_(game), 
    position_(position) 
{
    sphere_ = new SphereComponent();

    angle_ = 0.0;
    rotationSpeed_ = rand() / (float)RAND_MAX;

    rotation_ = {
        0.0,
        1.0,
        0.0
    };
}

PlanetComponent::~PlanetComponent() {
    delete sphere_;
}

void PlanetComponent::Initialize() {
    sphere_->Initialize();
}

void PlanetComponent::Update(float deltaTime) {
    angle_ += rotationSpeed_ * deltaTime;
}

void PlanetComponent::Draw() {
    auto scale = DirectX::SimpleMath::Matrix::CreateScale(115.0);
    auto rotation = DirectX::SimpleMath::Matrix::CreateRotationY(angle_);
    auto translation = DirectX::SimpleMath::Matrix::CreateTranslation(position_);

    auto matrix = translation;

    game_.UpdateModelBuffer(matrix);

    sphere_->Draw();
}

void PlanetComponent::Reload() {
    sphere_->Reload();
}

void PlanetComponent::DestroyResources() {
    sphere_->DestroyResources();
}
