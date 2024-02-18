#pragma once
#include <chrono>
#include <memory>
#include <vector>

#include <d3d.h>
#include <d3d11.h>

#include "GameComponent.h"

class Window;
class RenderContext;
class InputDevice;

class Game
{
private:
    static Game* instance_;
    std::vector<std::shared_ptr<GameComponent>> components_;

    Window* window_;
    RenderContext* renderCtx_;
    InputDevice* inputDevice_;

    float totalTime_;
    std::chrono::time_point<std::chrono::steady_clock> prevTime_;
    uint32_t frameCount_ = 0;

    bool isExitRequested_;

    void Initialize();
    void PrepareFrame();
    void Draw();
    void Update();
    void UpdateInternal();
    void EndFrame();
    void DestroyResources();

public:
    Game();

    void Run();
    void Exit();
    void PushComponent(std::shared_ptr<GameComponent>&& component);

    Window& GetWindow() {
        return *window_;
    }

    RenderContext& GetRenderContext() {
        return *renderCtx_;
    }

    InputDevice& GetInputDevice() {
        return *inputDevice_;
    }

    static Game& GetSingleton() {
        return *instance_;
    }
};

