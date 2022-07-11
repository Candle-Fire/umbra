
load("//projs/bazel-helpers:windows_dll_library.bzl", "windows_dll_library")

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
    shadow_game_bundle(
        name = name,
        data = [
            "//projs/shadow/shadow-runtime:shadow-runtime",
            name + "_lib.dll"
        ]
    )

    windows_dll_library(
        name = name + "_lib",
        srcs = srcs,
        hdrs = hdrs,
        # Define COMPILING_DLL to export symbols during compiling the DLL.
        copts = ["/DCOMPILING_DLL"],
    )