#include <string>
#include <vector>
#include <SimpleMath.h>

struct Texture {
    unsigned int id;
};  

struct Vertex {
    DirectX::SimpleMath::Vector3 Position;
    DirectX::SimpleMath::Vector3 Normal;
    DirectX::SimpleMath::Vector2 UV;
};

class MeshComponent : public GameComponent {
private:
    std::vector<Vertex> vertices_;
    std::vector<int> indices_;
    std::vector<Texture> textures_;

    ID3D11Buffer* vb_;
    ID3D11Buffer* ib_;

    ID3D11ShaderResourceView* texture_;
    ID3D11SamplerState* sampler_;
public:
    MeshComponent(std::vector<Vertex>&& vertices, std::vector<int>&& indices, std::vector<Texture>&& textures) : 
        vertices_(vertices), 
        indices_(indices), 
        textures_(textures), 
        GameComponent() 
    {
        vb_ = nullptr;
        ib_ = nullptr;
        texture_ = nullptr;
        sampler_ = nullptr;
    }

    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void Reload();
    virtual void DestroyResources();
}
