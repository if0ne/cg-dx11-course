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

	std::shared_ptr<SunSystemGame> game = std::make_shared<SunSystemGame>();

	Game::GetSingleton().PushComponent(std::move(game));

	Game::GetSingleton().Run();
}