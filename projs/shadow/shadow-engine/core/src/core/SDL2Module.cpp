//
// Created by dpete on 30/08/2022.
//

#include "core/SDL2Module.h"
#include "core/ShadowWindow.h"
#include "spdlog/spdlog.h"


void ShadowEngine::SDL2Module::Init() {
    // Initialize SDL. SDL_Init will return -1 if it fails.
    if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
        spdlog::error("Error creating window: " + std::string(SDL_GetError()));
        //system("pause");
        // End the program
        //return 1;
    }

    _window = new ShadowWindow(800,450);
}

void ShadowEngine::SDL2Module::PreInit() {

}

void ShadowEngine::SDL2Module::Update() {

}

void ShadowEngine::SDL2Module::Render() {

}

void ShadowEngine::SDL2Module::LateRender() {

}

std::string ShadowEngine::SDL2Module::GetName() {
    return this->GetType();
}

void ShadowEngine::SDL2Module::AfterFrameEnd() {

}
