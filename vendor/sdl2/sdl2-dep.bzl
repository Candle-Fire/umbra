load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

VERSION = "2.0.22"
URL_TOOLCHAIN = "https://www.libsdl.org/release/SDL2-devel-2.0.22-mingw.zip"

def sdl2_dep():
    if "sdl2" not in native.existing_rules():
        http_archive(
            name = "sdl2",
            build_file = Label("//vendor/sdl2:sdl2.BUILD"),
            url = URL_TOOLCHAIN,
            strip_prefix = "SDL2-2.0.22",
            sha256 = "18b3da2b3b5f1e802e0abbe99004d7404848755bbfe5153cd9dfcfc0a9522632",
        )