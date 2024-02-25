#pragma once
#include "../GameComponent.h"

#include "BallComponent.h"
#include "RacketComponent.h"
#include "DirectXCollision.h"

#include "DirectXCollision.h"

class PingPongGame : public GameComponent
{
private:
    RacketComponent* playerOne_;
    RacketComponent* playerTwo_;
    BallComponent* ball_;

    DirectX::BoundingBox goalPlayerOne_;
    DirectX::BoundingBox goalPlayerTwo_;

    DirectX::BoundingBox wallTop_;
    DirectX::BoundingBox wallBottom_;

    int playerOneScore;
    int playerTwoScore;

public:
    PingPongGame();
    ~PingPongGame();

    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void Reload();
    virtual void DestroyResources();
    
    void IncreasePlayerOneScore();
    void IncreasePlayerTwoScore();

    RacketComponent& GetRacketComponentPlayerOne() {
        return *playerOne_;
    }

    RacketComponent& GetRacketComponentPlayerTwo() {
        return *playerTwo_;
    }

    DirectX::BoundingBox GetGoalPlayerOne() {
        return goalPlayerOne_;
    }

    DirectX::BoundingBox GetGoalPlayerTwo() {
        return goalPlayerTwo_;
    }

    DirectX::BoundingBox GetWallTop() {
        return wallTop_;
    }

    DirectX::BoundingBox GetWallBottom() {
        return wallBottom_;
    }

};

