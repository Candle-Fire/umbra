
#include <VK2D/Structs.h>
#include <VK2D/Texture.h>

class RenderPipeline
{
public:
  virtual ~RenderPipeline() = default;

  virtual void Render();
};


class GameRenderer : public RenderPipeline
{
public:
  void Render() override
  {

  }
};

class EditorRenderer : public RenderPipeline
{
  VK2DTexture sceneTexture;
  GameRenderer& game_renderer;

public:
  EditorRenderer(GameRenderer& gameRenderer): game_renderer(gameRenderer)
  {
    sceneTexture = vk2dTextureCreate(1280, 720);
  }

  void Render() override
  {

  }
};