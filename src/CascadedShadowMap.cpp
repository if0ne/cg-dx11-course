#include "CascadedShadowMap.h"

#include "Camera.h"

CascadedShadowMapData CascadedShadowMap::CalcLightMatrices(const DirectX::SimpleMath::Vector3& lightDir, Camera& cam) {
	CascadedShadowMapData res;
	float curNear = cam.NearPlane();
	for (int i = 0; i < kCascadeCount; ++i)
	{
		float curFar = cam.FarPlane() * kCascadesFarRatios[i];

		const auto subFrustProj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(cam.Fov(), cam.AspectRatio(), curNear, curFar);

		curNear = curFar;

		auto corners = GetFrustumCornersWorldSpace(subFrustProj, cam.View());

		DirectX::SimpleMath::Vector3 center = DirectX::SimpleMath::Vector3::Zero;
		for (const auto& v : corners)
		{
			center += DirectX::SimpleMath::Vector3(v.x, v.y, v.z);
		}
		center /= corners.size();

		const auto lightView = DirectX::SimpleMath::Matrix::CreateLookAt(
			center,
			center + lightDir,
			DirectX::SimpleMath::Vector3::Up
		);

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();
		for (const auto& v : corners)
		{
			const auto trf = DirectX::SimpleMath::Vector4::Transform(v, lightView);
			minX = std::min(minX, trf.x);
			maxX = std::max(maxX, trf.x);
			minY = std::min(minY, trf.y);
			maxY = std::max(maxY, trf.y);
			minZ = std::min(minZ, trf.z);
			maxZ = std::max(maxZ, trf.z);
		}

		static constexpr float zMult = 10.0f;
		minZ = (minZ < 0) ? minZ * zMult : minZ / zMult;
		maxZ = (maxZ < 0) ? maxZ / zMult : maxZ * zMult;

		auto lightProj = DirectX::SimpleMath::Matrix::CreateOrthographicOffCenter(minX, maxX, minY, maxY, minZ, maxZ);

		res.viewProjMat[i] = lightView * lightProj;
		res.distances[i] = curFar;
	}

	return res;
}

std::vector<DirectX::SimpleMath::Vector4> CascadedShadowMap::GetFrustumCornersWorldSpace(const DirectX::SimpleMath::Matrix& proj, const DirectX::SimpleMath::Matrix& view) {
	const auto inv = (view * proj).Invert();

	std::vector<DirectX::SimpleMath::Vector4> frustumCorners;
	frustumCorners.reserve(8);
	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				const DirectX::SimpleMath::Vector4 pt =
					DirectX::SimpleMath::Vector4::Transform(DirectX::SimpleMath::Vector4(
						2.0f * static_cast<float>(x) - 1.0f,
						2.0f * static_cast<float>(y) - 1.0f,
						static_cast<float>(z),
						1.0f
					), inv);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}

	return frustumCorners;
}
