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
		auto lightOrthoMatrix = Matrix::CreateOrthographicOffCenter(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0001f, maxExtents.z - minExtents.z);
		
		//lightOrthoMatrix = Matrix::CreateOrthographic(100, 100, 0.0001, 1000);
		res.distances[i] = cam.NearPlane() + splitDist * clipRange;
		res.viewProjMat[i] = lightViewMatrix * lightOrthoMatrix;

		lastSplitDist = cam.NearPlane() + splitDist * clipRange;
	}

	return res;
}