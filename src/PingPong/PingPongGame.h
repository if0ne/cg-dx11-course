#pragma once
#include "../GameComponent.h"

#include "BallComponent.h"
#include "RacketComponent.h"

class PingPongGame : public GameComponent
{
private:
    RacketComponent* playerOne_;
    RacketComponent* playerTwo_;
    BallComponent* ball_;
public:
    PingPongGame();
    ~PingPongGame();

    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void Reload();
    virtual void DestroyResources();
};

