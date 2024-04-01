#include "Game.h"

#include "InputDevice.h"
#include "RenderContext.h"
#include "Window.h"
#include "AssetLoader.h"

Game* Game::instance_;

Game::Game() {
    instance_ = this;
    inputDevice_ = new InputDevice(*this);
    window_ = new Window(1240, 720);
    renderCtx_ = new RenderContext();
    assetLoader_ = new AssetLoader();

    components_ = {};

    isExitRequested_ = false;

    totalTime_ = 0.0;
    deltaTime_  = 0.0;
    prevTime_ = std::chrono::steady_clock::now();
}

Game::~Game() {
    delete renderCtx_;
    delete window_;
    delete inputDevice_;
}

void Game::Initialize() {
    renderCtx_->Initialize();
    window_->Initialize(*renderCtx_);
    inputDevice_->Initialize();

    for (auto& cmp : components_) {
        cmp->Initialize();
    }
}

void Game::ProcessInput() {
    window_->ProcessEvent();
}

void Game::PrepareFrame() {
    /*
    auto rt = window_->GetRenderTarget();
    auto ds = window_->GetDepthStencilView();

    renderCtx_->GetContext()->OMSetRenderTargets(1, &rt, ds);

    float color[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    renderCtx_->GetContext()->ClearRenderTargetView(rt, color);
    renderCtx_->GetContext()->ClearDepthStencilView(ds, D3D11_CLEAR_DEPTH, 1.0, 0);

    renderCtx_->ActivateDepthStencilState();
    */
}

void Game::SetViewport(int x, int y, float w, float h) {
    D3D11_VIEWPORT viewport = {};
    viewport.Width = w;
    viewport.Height = h;
    viewport.TopLeftX = x;
    viewport.TopLeftY = y;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1.0f;

    renderCtx_->GetContext()->RSSetViewports(1, &viewport);
}

void Game::Draw() {
    for (auto& comp : components_) {
        comp->Draw();
    }
}

void Game::Update() {
    for (auto& comp : components_) {
        comp->Update(deltaTime_);
    }
}

void Game::UpdateInternal() {
    auto curTime = std::chrono::steady_clock::now();
    deltaTime_ = std::chrono::duration_cast<std::chrono::microseconds>(curTime - prevTime_).count() / 1000000.0f;
    prevTime_ = curTime;

    totalTime_ += deltaTime_;
    frameCount_++;

    if (totalTime_ > 1.0f) {
        float fps = frameCount_ / totalTime_;

        totalTime_ -= 1.0f;

        WCHAR text[256];
        swprintf_s(text, TEXT("FPS: %f"), fps);
        SetWindowText(window_->GetDescriptor(), text);

        frameCount_ = 0;
    }
}

void Game::EndFrame() {
    renderCtx_->GetContext()->OMSetRenderTargets(0, nullptr, nullptr);
    window_->GetSwapchain()->Present(1, 0);
}

void Game::DestroyResources() {
    for (auto& cmp : components_) {
        cmp->DestroyResources();
    }

    window_->DestroyResources();
    renderCtx_->DestroyResources();
}

void Game::Run() {
    Initialize();
    window_->Show();

    while (!isExitRequested_) {
        UpdateInternal();

        ProcessInput();
        Update();
        PrepareFrame();
        SetViewport(0, 0, window_->GetWidth(), window_->GetHeight());
        Draw();
        EndFrame();
    }

    DestroyResources();
}

void Game::PushComponent(std::shared_ptr<GameComponent>&& component) {
    components_.push_back(component);
}

void Game::Exit() {
    isExitRequested_ = true;
}
