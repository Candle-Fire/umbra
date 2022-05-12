load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")

VERSION = "2.0.22"
URL_TOOLCHAIN = "https://github.com/ocornut/imgui.git"

def glm_lib():
    if "glm_lib" not in native.existing_rules():
        new_git_repository(
            name = "glm_lib",
            remote = "https://github.com/g-truc/glm.git",
            commit = "658d8960d081e0c9c312d49758c7ef919371b428",
            build_file = Label("//vendor/glm:glm.BUILD"),
        )