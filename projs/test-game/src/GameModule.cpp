#include <GameModule.h>
#include <iostream>

void GameModule::PreInit() { std::cout << "what?" << std::endl; }

void GameModule::Init() {}

void GameModule::Update() {}

void GameModule::Render() {}

void GameModule::LateRender() {}

void GameModule::AfterFrameEnd() {}