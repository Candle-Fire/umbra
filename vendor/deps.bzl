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

    http_archive(
        name = "bazel_skylib",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.2.1/bazel-skylib-1.2.1.tar.gz",
            "https://github.com/bazelbuild/bazel-skylib/releases/download/1.2.1/bazel-skylib-1.2.1.tar.gz",
        ],
        sha256 = "f7be3474d42aae265405a592bb7da8e171919d74c16f082a5457840f06054728",
    )