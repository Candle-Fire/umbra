#include "shadow/core/ShadowWindow.h"
#include <spdlog/spdlog.h>

#include <string>

SH::Window::Window(int W, int H) : Height(H), Width(W)
{
}

SH::Window::~Window() = default;
