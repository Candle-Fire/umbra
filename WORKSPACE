load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

workspace(
    name = "umbra",
)

load("//vendor:deps.bzl", "deps")

deps()

#########################################################################
######################## Vulkan #########################################
#########################################################################

git_repository(
    name = "rules_vulkan",
    remote = "https://github.com/jadarve/rules_vulkan.git",
    tag = "v0.0.6"
)

load("@rules_vulkan//vulkan:repositories.bzl", "vulkan_repositories")

vulkan_repositories()

#########################################################################
######################## Bazel lib ######################################
#########################################################################

load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")
bazel_skylib_workspace()