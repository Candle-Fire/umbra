load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("//vendor/sdl2:sdl2-dep.bzl", "sdl2_dep")
load("//vendor/glm:glm-dep.bzl", "glm_dep")


def deps():
    sdl2_dep()
    glm_dep()

    # this doesn't require a whole folder with a build, props, bzl file, so it's set here.
    http_archive(
        name = "catch2",
        strip_prefix = "Catch2-2.13.0",
        urls = ["https://github.com/catchorg/Catch2/archive/v2.13.0.tar.gz"],
    )