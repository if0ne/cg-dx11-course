#include "PlanetComponent.h"

#include "../SphereComponent.h"

#include "SunSystemGame.h"
#include "SatelliteComponent.h"

using namespace DirectX::SimpleMath;

PlanetComponent::PlanetComponent(
    SunSystemGame& game,
    float radius,
    float size,
    int satelliteCount,
    DirectX::SimpleMath::Vector3 firstColor,
    DirectX::SimpleMath::Vector3 secondColor
) :
    game_(game)
{
    sphere_ = new SphereComponent(firstColor, secondColor);

    localAngle_ = 0.0;
    globalAngle_ = 0.0;
    localRotationSpeed_ = rand() / (float)RAND_MAX * 10.0;
    globalRotationSpeed_ = rand() / (float)RAND_MAX;

    scale_ = size;
    localPosition_ = Vector3(0.0, 0.0, radius);
    globalPosition_ = Vector3(0.0, 0.0, radius);

    localAxisRotation_ = Vector3(
        rand() / (float)RAND_MAX,
        rand() / (float)RAND_MAX,
        rand() / (float)RAND_MAX
    );

    localAxisRotation_.Normalize();

    for (int i = 0; i < satelliteCount; i++) {
        float radius = (rand() / (float)RAND_MAX + 1.0) * scale_ * 2.0;
        float size = (rand() / (float)RAND_MAX + 1) * scale_ / 10;
        satellites_.push_back(new SatelliteComponent(*this, game_, radius, size));
    }
}

PlanetComponent::~PlanetComponent() {
    delete sphere_;

    for (auto& sat : satellites_) {
        delete sat;
    }
}

void PlanetComponent::Initialize() {
    sphere_->Initialize();

    for (auto& sat : satellites_) {
        sat->Initialize();
    }
}

void PlanetComponent::Update(float deltaTime) {
    localAngle_ += localRotationSpeed_ * deltaTime;
    globalAngle_ += globalRotationSpeed_ * deltaTime;

    auto rotQuat = Quaternion::CreateFromAxisAngle(Vector3::Up, globalAngle_);
    auto rotMat = Matrix::CreateFromQuaternion(rotQuat);

    globalPosition_ = Vector3::Transform(localPosition_, rotQuat);

    for (auto& sat : satellites_) {
        sat->Update(deltaTime);
    }
}

void PlanetComponent::Draw() {
    auto scale = Matrix::CreateScale(scale_);
    auto rotQuat = Quaternion::CreateFromAxisAngle(localAxisRotation_, localAngle_);
    auto rotation = Matrix::CreateFromQuaternion(rotQuat);
    auto translation = Matrix::CreateTranslation(globalPosition_);

    auto matrix = scale * rotation * translation;

    game_.UpdateModelBuffer(matrix);

    sphere_->Draw();

    for (auto& sat : satellites_) {
        sat->Draw();
    }
}

void PlanetComponent::Reload() {
    sphere_->Reload();

    for (auto& sat : satellites_) {
        sat->Reload();
    }
}

void PlanetComponent::DestroyResources() {
    sphere_->DestroyResources();

    for (auto& sat : satellites_) {
        sat->DestroyResources();
    }
}
