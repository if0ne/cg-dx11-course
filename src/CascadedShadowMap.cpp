#include "CascadedShadowMap.h"

#include "Camera.h"

using namespace DirectX::SimpleMath;

CascadedShadowMapData CascadedShadowMap::CalcLightMatrices(const Vector3& lightDir, Camera& cam) {
	CascadedShadowMapData res;

	float curNear = cam.NearPlane();

	const auto projMatrix = Matrix::CreatePerspectiveFieldOfView(cam.Fov(), cam.AspectRatio(), curNear, cam.FarPlane());
	const auto viewMatrix = cam.View();

	for (int i = 0; i < kCascadeCount; ++i)
	{
		float curFar = cam.FarPlane() * kCascadesFarRatios[i];
		curNear = curFar;

		Vector3 corners[8] = {
			Vector3(-1.0f, 1.0f, -1.0f),
			Vector3(1.0f, 1.0f, -1.0f),
			Vector3(1.0f, -1.0f, -1.0f),
			Vector3(-1.0f, -1.0f, -1.0f),
			Vector3(-1.0f, 1.0f, 1.0f),
			Vector3(1.0f, 1.0f, 1.0f),
			Vector3(1.0f, -1.0f, 1.0f),
			Vector3(-1.0f, -1.0f, 1.0f)
		};

		auto invCam = (viewMatrix * projMatrix).Invert();

		for (int j = 0; j < 8; j++) {
			Vector4 invCorner = Vector4(corners[j]);
			invCorner.w = 1.0;
			invCorner = Vector4::Transform(invCorner, invCam);
			corners[j] = Vector3(invCorner.x / invCorner.w, invCorner.y / invCorner.w, invCorner.z / invCorner.w);
		}

		Vector3 center = Vector3::Zero;
		for (const auto& v : corners)
		{
			center += v;
		}
		center /= 8.0;

		float radius = 0.0f;
		for (int j = 0; j < 8; j++) {
			float distance = (corners[j] - center).Length();
			radius = std::max(radius, distance);
		}
		radius = std::ceil(radius * 16.0f) / 16.0f;

		auto maxExtends = Vector3(radius);
		auto minExtends = maxExtends * -1.0;

		auto lightView = DirectX::SimpleMath::Matrix::CreateLookAt(
			center,
			center + lightDir,
			DirectX::SimpleMath::Vector3::Up
		);

		auto lightProj = Matrix::CreateOrthographicOffCenter(minExtends.x, maxExtends.x, minExtends.y, maxExtends.y, 0.0, maxExtends.z - minExtends.z);

		res.viewProjMat[i] = lightView * lightProj;
		res.distances[i] = curFar;
	}

	return res;
}
