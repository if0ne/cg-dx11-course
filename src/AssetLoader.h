#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <unordered_map>

class ModelComponent;

class AssetLoader {
private:
    Assimp::Importer importer_;
    std::unordered_map<std::string, ModelComponent> loadedModels_;
public:
    AssetLoader() {}

    ModelComponent& LoadModel(std::string& path);
}
