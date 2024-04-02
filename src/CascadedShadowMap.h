#pragma once

#include <SimpleMath.h>

#include <vector>

class Camera;

struct CascadedShadowMapData {
    DirectX::SimpleMath::Matrix viewProjMat[4];
    float distances[4];
};

class CascadedShadowMap
{
private:
    const int kCascadeCount = 4;
    const float kCascadesFarRatios[4] = { 0.2,0.4,0.6,1.0 };
    const int kWidth = 2048;
    const int kHeight = 2048;

    DirectX::SimpleMath::Matrix lightProj_;
public:
    CascadedShadowMap() {
        lightProj_ = DirectX::SimpleMath::Matrix::CreateOrthographic(100, 100, 0.0001, 1000);
    }

    CascadedShadowMapData RenderData(const DirectX::SimpleMath::Vector3& lightDir, Camera& cam) {
		CalcLightMatrices(lightDir, cam);
    }

private:
	CascadedShadowMapData CalcLightMatrices(const DirectX::SimpleMath::Vector3& lightDir, Camera& cam);

    std::vector<DirectX::SimpleMath::Vector4> GetFrustumCornersWorldSpace(const DirectX::SimpleMath::Matrix& proj, const DirectX::SimpleMath::Matrix& view);
};

