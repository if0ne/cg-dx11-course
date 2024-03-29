#pragma once
class Game;

class GameComponent
{
protected:
    Game& ctx_;
    GameComponent();

public:
    virtual void Initialize() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;
    virtual void Reload() = 0;
    virtual void DestroyResources() = 0;
};

