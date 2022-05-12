load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

VERSION = "2.0.22"
URL_TOOLCHAIN = "https://github.com/ocornut/imgui.git"

def imgui_lib():
    if "imgui_lib" not in native.existing_rules():
        git_repository(
            name = "imgui_lib",
            #build_file = Label("//vendor/imgui:imgui.BUILD"),
            remote = URL_TOOLCHAIN,
            tag = "v0.0.6",
        )