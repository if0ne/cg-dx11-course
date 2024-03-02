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
    auto center = DirectX::SimpleMath::Vector3::Zero - position_;
    auto quat = DirectX::SimpleMath::Matrix::CreateFromQuaternion(DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(rotation_, angle_));

    auto rotated = DirectX::SimpleMath::Vector3::Transform(center, quat);

    auto matrix = DirectX::SimpleMath::Matrix::CreateTranslation(-rotated);
    game_.UpdateModelBuffer(matrix);

    sphere_->Draw();
}

void PlanetComponent::Reload() {
    sphere_->Reload();
}

void PlanetComponent::DestroyResources() {
    sphere_->DestroyResources();
}
