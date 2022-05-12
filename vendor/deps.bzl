load("//vendor/sdl2:sdl2-lib.bzl", "sdl2Lib")
load("//vendor/gtest:gtest-dep.bzl", "gtest_dep")
#load("//vendor/vulkan:vulkan_dep.bzl", "vulkan_dep")
load("//vendor/imgui:imgui_lib.bzl", "imgui_lib")
load("//vendor/glm:glm_lib.bzl", "glm_lib")

def deps():
    sdl2Lib()
    gtest_dep()
    imgui_lib()
    glm_lib()