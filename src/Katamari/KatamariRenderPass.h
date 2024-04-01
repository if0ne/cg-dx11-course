#pragma once
#include "../RenderPass.h"

class KatamariGame;

class KatamariRenderPass : public RenderPass
{
private:
    KatamariGame& game_;
public:
    KatamariRenderPass(
        std::string&& shaderPath,
        std::vector<std::pair<const char*, DXGI_FORMAT>>&& vertexAttr,
        KatamariGame& game
    );

    virtual void Execute();
};

