#pragma once
class Game;

class GameComponent
{
protected:
    Game& ctx_;
    GameComponent(Game& ctx) : ctx_(ctx) {}

public:
    virtual void Initialize() = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual void Reload() = 0;
    virtual void DestroyResources() = 0;
};

