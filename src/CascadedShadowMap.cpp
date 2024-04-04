#include "CascadedShadowMap.h"

#include "Camera.h"

using namespace DirectX::SimpleMath;

CascadedShadowMapData CascadedShadowMap::CalcLightMatrices(const Vector3& lightDir, Camera& cam) {
	CascadedShadowMapData res;

	float curNear = cam.NearPlane();
	float curFar = cam.FarPlane();

	float clipRange = curFar - curNear;

	float minZ = curNear;
	float maxZ = curNear + clipRange;

	float range = maxZ - minZ;
	float ratio = maxZ / minZ;

	for (uint32_t i = 0; i < kCascadeCount; i++) {
		float p = (i + 1) / static_cast<float>(kCascadeCount);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = 0.5 * (log - uniform) + uniform;
		cascadesFarRatios[i] = (d - curNear) / clipRange;
	}

	float lastSplitDist = curNear;
	for (int i = 0; i < kCascadeCount; ++i) {
		float splitDist = cascadesFarRatios[i];

		Vector3 frustumCorners[8] = {
			Vector3(-1.0f, -1.0f, 0.0f),
			Vector3(-1.0f, -1.0f, 1.0f),
			Vector3(-1.0f, 1.0f, 0.0f),
			Vector3(-1.0f, 1.0f, 1.0f),
			Vector3(1.0f, -1.0f, 0.0f),
			Vector3(1.0f, -1.0f, 1.0f),
			Vector3(1.0f, 1.0f, 0.0f),
			Vector3(1.0f, 1.0f, 1.0f),
		};
		auto camView = cam.View();
		auto frustProj = Matrix::CreatePerspectiveFieldOfView(cam.Fov(), cam.AspectRatio(), lastSplitDist, curNear + curFar * splitDist);
		auto invCam = (camView * frustProj).Invert();

		for (int j = 0; j < 8; j++) {
			auto invCorner = Vector4::Transform(Vector4(frustumCorners[j].x, frustumCorners[j].y, frustumCorners[j].z, 1.0f), invCam);
			invCorner /= invCorner.w;
			frustumCorners[j].x = invCorner.x;
			frustumCorners[j].y = invCorner.y;
			frustumCorners[j].z = invCorner.z;
		}

		// Get frustum center
		Vector3 frustumCenter = Vector3::Zero;
		for (uint32_t j = 0; j < 8; j++) {
			frustumCenter += frustumCorners[j];
		}
		frustumCenter /= 8.0f;

		float radius = 0.0f;
		for (uint32_t j = 0; j < 8; j++) {
			float distance = (frustumCorners[j] - frustumCenter).Length();
			radius = std::max(radius, distance);
		}
		radius = std::ceil(radius * 16.0f) / 16.0f;

		auto maxExtents = Vector3(radius, radius, radius);
		auto minExtents = -maxExtents;

		auto lightViewMatrix = Matrix::CreateLookAt(
			frustumCenter - lightDir, 
			frustumCenter, 
			Vector3::Up
		);

		/*float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();
		for (const auto& v : frustumCorners)
		{
			const auto trf = DirectX::SimpleMath::Vector3::Transform(v, lightViewMatrix);
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

		auto lightOrthoMatrix = DirectX::SimpleMath::Matrix::CreateOrthographicOffCenter(minX, maxX, minY, maxY, minZ, maxZ);*/

		auto lightOrthoMatrix = Matrix::CreateOrthographicOffCenter(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0001f, maxExtents.z - minExtents.z);
		
		//lightOrthoMatrix = Matrix::CreateOrthographic(100, 100, 0.0001, 1000);
		res.distances[i] = cam.NearPlane() + splitDist * clipRange;
		res.viewProjMat[i] = lightViewMatrix * lightOrthoMatrix;

		lastSplitDist = cam.NearPlane() + splitDist * clipRange;
	}

	return res;
}