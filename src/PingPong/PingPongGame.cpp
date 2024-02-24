#include "PingPongGame.h"

#include "BallComponent.h"
#include "../Keys.h"

PingPongGame::PingPongGame() {
    playerOne_ = new RacketComponent(-0.1, 0.0, Keys::W, Keys::S);
    playerTwo_ = new RacketComponent(0.1, 0.0, Keys::Up, Keys::Down);
    ball_ = new BallComponent();
}

PingPongGame::~PingPongGame() {
    delete playerOne_;
    delete playerTwo_;
    delete ball_;
}

void PingPongGame::Initialize() {
    playerOne_->Initialize();
    playerTwo_->Initialize();
    ball_->Initialize();
}

void PingPongGame::Update(float deltaTime) {
    playerOne_->Update(deltaTime);
    playerTwo_->Update(deltaTime);
    ball_->Update(deltaTime);
}

void PingPongGame::Draw() {
    playerOne_->Draw();
    playerTwo_->Draw();
    ball_->Draw();
}

void PingPongGame::Reload() {
    playerOne_->Reload();
    playerTwo_->Reload();
    ball_->Reload();
}

void PingPongGame::DestroyResources() {
    playerOne_->DestroyResources();
    playerTwo_->DestroyResources();
    ball_->DestroyResources();
}