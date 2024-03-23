#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <unordered_map>

#include "ModelComponent.h"

class AssetLoader
{
private:
    const std::string kDirectory = "./assets";

    Assimp::Importer importer_;
    std::unordered_map<std::string, ModelComponent> loadedModels_;
public:
    AssetLoader() {}

    ModelComponent& LoadModel(std::string& path);
};


