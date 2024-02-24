#include "PingPongGame.h"

#include "../Keys.h"

#include <iostream>

#include "BallComponent.h"

PingPongGame::PingPongGame() {
    playerOne_ = new RacketComponent(-0.8, 0.0, Keys::W, Keys::S, *this);
    playerTwo_ = new RacketComponent(0.75, 0.0, Keys::Up, Keys::Down, *this);
    ball_ = new BallComponent(*this);

    playerOneScore = 0;
    playerTwoScore = 0;

    wallTop_.Center.x = 0.0;
    wallTop_.Center.y = 1.5;
    wallTop_.Extents.x = 2.0;
    wallTop_.Extents.y = 0.5;

    wallBottom_.Center.x = 0.0;
    wallBottom_.Center.y = -1.5;
    wallBottom_.Extents.x = 2.0;
    wallBottom_.Extents.y = 0.5;

    goalPlayerOne_.Center.x = -1.5;
    goalPlayerOne_.Center.y = 0.0;
    goalPlayerOne_.Extents.x = 0.5;
    goalPlayerOne_.Extents.y = 2.0;

    goalPlayerTwo_.Center.x = 1.5;
    goalPlayerTwo_.Center.y = 0.0;
    goalPlayerTwo_.Extents.x = 0.5;
    goalPlayerTwo_.Extents.y = 2.0;
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

void PingPongGame::IncreasePlayerOneScore() {
    playerOneScore++;
    std::cout << playerOneScore << " : " << playerTwoScore << std::endl;
}

void PingPongGame::IncreasePlayerTwoScore() {
    playerTwoScore++;
    std::cout << playerOneScore << " : " << playerTwoScore << std::endl;
}
