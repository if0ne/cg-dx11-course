#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include "Game.h"
#include "SquareComponent.h"

#include <memory>

int main()
{
	std::shared_ptr<SquareComponent> cmp1 = std::make_shared<SquareComponent>(0.0);
	std::shared_ptr<SquareComponent> cmp2 = std::make_shared<SquareComponent>(0.25);

	Game::GetSingleton().PushComponent(std::move(cmp1));
	Game::GetSingleton().PushComponent(std::move(cmp2));

	Game::GetSingleton().Run();
}