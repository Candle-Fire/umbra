#pragma once

#include <memory>

#include "module_manager.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "imgui.h"


class IModule
{
protected:
  IModule() {}
public:
  virtual ~IModule() = default;
  virtual void Init() = 0;
  virtual void Update() = 0;

  virtual void StartRender() {};
  virtual void Render() = 0;
  virtual void ProcessEvent(SDL_Event& event) = 0;
};

void Init();
void Update();
void StartRender();
void Render();

void ProcessEvent(SDL_Event& event);


class SDL3Module : public IModule
{
public:
  SDL_Window *window = nullptr;
  float window_scale = 0;

  explicit SDL3Module() {};
  void Init() override;
  void Update() override;
  void Render() override {};

private:
  void ProcessEvent(SDL_Event& event) override {}
};
extern std::shared_ptr<SDL3Module> sdl3_module;


class ImGUIModule : public IModule
{
  bool show_demo_window = true;
  bool show_another_window = false;

public:
  ImGUIModule() = default;

  void Init() override;
  void ProcessEvent(SDL_Event& event) override;
  void Update() override;
  void Render() override;

private:
  void DrawUI();
};
extern std::shared_ptr<ImGUIModule> imgui_module;


class Renderer : public IModule
{
public:
  SDL_GPUDevice* device;
  SDL_Renderer* renderer2d;

  SDL_Texture* game_target;

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  Renderer() = default;

  void Init() override;
  void Update() override;
  void ProcessEvent(SDL_Event& event) override;
  void StartRender() override;
  void Render() override;
};
extern std::shared_ptr<Renderer> renderer;
