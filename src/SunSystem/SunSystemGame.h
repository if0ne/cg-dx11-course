#pragma once
#include "../GameComponent.h"

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <SimpleMath.h>

class SunSystemGame : public GameComponent
{
    DirectX::SimpleMath::Matrix a;
};

