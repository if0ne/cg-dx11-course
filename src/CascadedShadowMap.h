#pragma once

#include <SimpleMath.h>

#include <vector>

class Camera;

struct CascadedShadowMapData {
    DirectX::SimpleMath::Matrix viewProjMat[4];
    float distances[4];
};

struct CascadeData {
    DirectX::SimpleMath::Matrix viewProjMat;
};

class CascadedShadowMap
{
private:
    DirectX::SimpleMath::Matrix lightProj_;
public:
    const int kCascadeCount = 4;
    const float kCascadesFarRatios[4] = { 0.1, 0.25, 0.5,1.0 };
    const int kWidth = 2048;
    const int kHeight = 2048;

    CascadedShadowMap() {
        lightProj_ = DirectX::SimpleMath::Matrix::CreateOrthographic(100, 100, 0.0001, 1000);
    }

    CascadedShadowMapData RenderData(const DirectX::SimpleMath::Vector3& lightDir, Camera& cam) {
		return CalcLightMatrices(lightDir, cam);
    }

private:
	CascadedShadowMapData CalcLightMatrices(const DirectX::SimpleMath::Vector3& lightDir, Camera& cam);

    std::vector<DirectX::SimpleMath::Vector4> GetFrustumCornersWorldSpace(const DirectX::SimpleMath::Matrix& proj, const DirectX::SimpleMath::Matrix& view);
};

