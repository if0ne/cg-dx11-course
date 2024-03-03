#include "SatelliteComponent.h"

#include "SunSystemGame.h"
#include "PlanetComponent.h"

#include "../CubeComponent.h"

using namespace DirectX::SimpleMath;

SatelliteComponent::SatelliteComponent(PlanetComponent& parent, SunSystemGame& game, float radius, float size) : 
    parent_(parent), 
    game_(game) 
{
    cube_ = new CubeComponent();

    globalAngle_ = 0.0;
    globalRotationSpeed_ = rand() / (float)RAND_MAX * 5.0;

    scale_ = size;
    localPosition_ = Vector3(0.0, 0.0, radius);
    globalPosition_ = parent_.GetGlobalPosition() + localPosition_;

    globalAxisRotation_ = Vector3(
        rand() / (float)RAND_MAX,
        rand() / (float)RAND_MAX,
        rand() / (float)RAND_MAX
    );
}

SatelliteComponent::~SatelliteComponent() {
    delete cube_;
}

void SatelliteComponent::Initialize() {
    cube_->Initialize();
}

void SatelliteComponent::Update(float deltaTime) {
    globalAngle_ += globalRotationSpeed_ * deltaTime;

    auto rotQuat = Quaternion::CreateFromAxisAngle(Vector3::Up, globalAngle_);
    auto rotMat = Matrix::CreateFromQuaternion(rotQuat);

    globalPosition_ = Vector3::Transform(localPosition_, rotQuat) + parent_.GetGlobalPosition();

}

void SatelliteComponent::Draw() {
    auto scale = Matrix::CreateScale(scale_);
    auto translation = Matrix::CreateTranslation(globalPosition_);

    auto matrix = scale * translation;

    game_.UpdateModelBuffer(matrix);

    cube_->Draw();
}

void SatelliteComponent::Reload() {
    cube_->Reload();
}

void SatelliteComponent::DestroyResources() {
    cube_->DestroyResources();
}
