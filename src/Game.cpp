#include "Game.h"

#include "InputDevice.h"
#include "RenderContext.h"
#include "Window.h"

Game* Game::instance_;

Game::Game() {
    instance_ = this;
    inputDevice_ = new InputDevice(*this);
    window_ = new Window(800, 600);
    renderCtx_ = new RenderContext();

    components_ = {};

    isExitRequested_ = false;

    totalTime_ = 0.0;
    deltaTime_  = 0.0;
    prevTime_ = std::chrono::steady_clock::now();
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
    auto rt = window_->GetRenderTarget();
    renderCtx_->GetContext()->OMSetRenderTargets(1, &rt, nullptr);

    float color[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    renderCtx_->GetContext()->ClearRenderTargetView(rt, color);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(window_->GetWidth());
    viewport.Height = static_cast<float>(window_->GetHeight());
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
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