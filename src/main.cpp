#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include "Game.h"
#include "SquareComponent.h"

#include <memory>

int main()
{
	std::shared_ptr<SquareComponent> cmp = std::make_shared<SquareComponent>();

	Game::GetSingleton().PushComponent(std::move(cmp));

	Game::GetSingleton().Run();
}