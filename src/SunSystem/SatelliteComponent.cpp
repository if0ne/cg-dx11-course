#include "SatelliteComponent.h"

#include "PlanetComponent.h"

SatelliteComponent::SatelliteComponent(PlanetComponent& parent) : parent_(parent) {
}

SatelliteComponent::~SatelliteComponent() {
}

void SatelliteComponent::Initialize() {
}

void SatelliteComponent::Update(float deltaTime) {
}

void SatelliteComponent::Draw() {
}

void SatelliteComponent::Reload() {
}

void SatelliteComponent::DestroyResources() {
}
