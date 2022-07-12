
load("//projs/bazel-helpers:windows_dll_library.bzl", "windows_dll_library")
load("//projs/bazel-helpers:copy_to_directory.bzl", "copy_to_directory")

def _foo_binary_impl(ctx):
    trans_srcs = depset(transitive = [ dep[DefaultInfo].files for dep in ctx.attr.data])
    return [
        DefaultInfo(files = trans_srcs),
    ]
    

shadow_game_bundle = rule(
    implementation = _foo_binary_impl,
    attrs = {
        "data": attr.label_list(),
    },
)

def shadow_game(
        name,
        srcs = [],
        deps = [],
        hdrs = [],
        visibility = None,
        **kwargs):
    copy_to_directory(
        name = name + "-game",
        srcs = [
            "//projs/shadow/shadow-runtime",
            name + "_lib.dll",
        ],
    )

    windows_dll_library(
        name = name + "_lib",
        srcs = srcs,
        hdrs = hdrs,
        # Define COMPILING_DLL to export symbols during compiling the DLL.
        copts = ["/DCOMPILING_DLL"],
    )