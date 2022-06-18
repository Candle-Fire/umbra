load("//vendor/sdl2:sdl2-dep.bzl", "sdl2_dep")
load("//vendor/gtest:gtest-dep.bzl", "gtest_dep")
#load("//vendor/imgui:imgui_lib.bzl", "imgui_lib")
load("//vendor/glm:glm-dep.bzl", "glm_dep")

def deps():
    sdl2_dep()
    gtest_dep()
#    imgui_lib()
    glm_dep()