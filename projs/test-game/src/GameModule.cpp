#include <GameModule.h>
#include <iostream>
#include "imgui.h"
#include "core/Time.h"

void GameModule::PreInit() { std::cout << "what?" << std::endl; }

void GameModule::Init() {}

void GameModule::Update() {}

void GameModule::Render() {

    bool active = true;
    ImGui::Begin("Game module window", &active, ImGuiWindowFlags_MenuBar);

    ImGui::Text("Such teext");

    ImGui::End();


}

void GameModule::LateRender() {}

void GameModule::AfterFrameEnd() {}