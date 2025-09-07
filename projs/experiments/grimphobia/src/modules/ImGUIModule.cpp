//
// Created by dpeter99 on 06/09/2025.
//

#include "modules.h"

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlgpu3.h"

void ImGUIModule::Init()
{
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsLight();

  // Setup scaling
  ImGuiStyle& style = ImGui::GetStyle();
  style.ScaleAllSizes(sdl3_module->window_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
  style.FontScaleDpi = sdl3_module->window_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

  // Setup Platform/Renderer backends
  ImGui_ImplSDL3_InitForSDLGPU(sdl3_module->window);
  ImGui_ImplSDLGPU3_InitInfo init_info = {};
  init_info.Device = renderer->device;
  init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(renderer->device, sdl3_module->window);
  init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;                      // Only used in multi-viewports mode.
  init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;  // Only used in multi-viewports mode.
  init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
  ImGui_ImplSDLGPU3_Init(&init_info);

}

void ImGUIModule::ProcessEvent(SDL_Event& event)
{
  ImGui_ImplSDL3_ProcessEvent(&event);
}

void ImGUIModule::Update()
{
  // Start the Dear ImGui frame
  ImGui_ImplSDLGPU3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  DrawUI();

  ImGui::Begin("Scene");
  ImGui::Image(renderer->game_target,ImVec2{100,100});
  ImGui::End();
}

void ImGUIModule::Render()
{
  // Rendering
  ImGui::Render();
  // ImDrawData* draw_data = ImGui::GetDrawData();
  // const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
  //
  // SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(renderer->device); // Acquire a GPU command buffer
  //
  // SDL_GPUTexture* swapchain_texture;
  // SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, sdl3_module->window, &swapchain_texture, nullptr, nullptr); // Acquire a swapchain texture
  //
  // if (swapchain_texture != nullptr && !is_minimized)
  // {
  //   // This is mandatory: call ImGui_ImplSDLGPU3_PrepareDrawData() to upload the vertex/index buffer!
  //   ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);
  //
  //   // Setup and start a render pass
  //   SDL_GPUColorTargetInfo target_info = {};
  //   target_info.texture = swapchain_texture;
  //   target_info.clear_color = SDL_FColor { renderer->clear_color.x, renderer->clear_color.y, renderer->clear_color.z, renderer->clear_color.w };
  //   target_info.load_op = SDL_GPU_LOADOP_CLEAR;
  //   target_info.store_op = SDL_GPU_STOREOP_STORE;
  //   target_info.mip_level = 0;
  //   target_info.layer_or_depth_plane = 0;
  //   target_info.cycle = false;
  //   SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &target_info, 1, nullptr);
  //
  //   // Render ImGui
  //   ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, render_pass);
  //
  //   SDL_EndGPURenderPass(render_pass);
  // }
  //
  // // Submit the command buffer
  // SDL_SubmitGPUCommandBuffer(command_buffer);


}

void ImGUIModule::DrawUI()
{
  // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
  if (show_demo_window)
    ImGui::ShowDemoWindow(&show_demo_window);

  // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
  {
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
    ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
    ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color", (float*)&renderer->clear_color); // Edit 3 floats representing a color

    if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
      counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
  }

  // 3. Show another simple window.
  if (show_another_window)
  {
    ImGui::Begin("Another Window", &show_another_window);
    // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    ImGui::Text("Hello from another window!");
    if (ImGui::Button("Close Me"))
      show_another_window = false;
    ImGui::End();
  }

}


