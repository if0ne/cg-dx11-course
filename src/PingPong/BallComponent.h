#pragma once
#include "../GameComponent.h"

class BallComponent : public GameComponent
{
public:
    BallComponent();

    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void Reload();
    virtual void DestroyResources();
};

