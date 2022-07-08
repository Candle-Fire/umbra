load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "shadow-utility",
    srcs = glob(["**/*.cpp", "**/*.h"]),
    hdrs = glob(["**/*.h"]),
    strip_include_prefix = "src/",
    includes = [],
    copts = [
        "/std:c++20"
    ],
    deps = [
    ],
    visibility = ["//visibility:public"],
)

