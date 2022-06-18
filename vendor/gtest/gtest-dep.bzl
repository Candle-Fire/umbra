load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")


def gtest_dep():
    if "gtest" not in native.existing_rules():
        git_repository(
            name = "gtest",
            remote = "https://github.com/google/googletest",
            branch = "v1.10.x",
        )