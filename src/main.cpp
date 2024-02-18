#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include "Game.h"
#include "TriangleComponent.h"

#include <memory>

int main()
{
	Game game{};

	std::shared_ptr<TriangleComponent> cmp = std::make_shared<TriangleComponent>(game);

	game.PushComponent(std::move(cmp));

	game.Run();
}