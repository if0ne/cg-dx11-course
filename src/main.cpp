#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include "Game.h"
#include "Katamari/KatamariGame.h"

#include <memory>

int main()
{
	srand(time(0));

	auto katamari = std::make_shared(KatamariGame());

	Game::GetSingleton().PushComponent(std::move(katamari));
	Game::GetSingleton().Run();
}
