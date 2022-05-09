load("//vendor/sdl2:sdl2-lib.bzl", "sdl2Lib")
load("//vendor/gtest:gtest-dep.bzl", "gtest_dep")
#load("//vendor/vulkan:vulkan_dep.bzl", "vulkan_dep")

def deps():
    sdl2Lib()
    gtest_dep()
    #vulkan_dep()