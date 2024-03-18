#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include "Game.h"
#include "SunSystem/SunSystemGame.h"

#include <memory>

int main()
{
	srand(time(0));

	Game::GetSingleton().Run();
}