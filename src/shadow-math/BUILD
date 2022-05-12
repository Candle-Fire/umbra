load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "shadow-math",
    srcs = glob([
        "**/*.cpp",
        "**/*.h"
    ]),
    hdrs = [
        "Transform.h"
    ],
    include_prefix = "shadow-math",
    deps = [
        "@glm_lib//:glm"
    ],
    copts = ["/std:c++17"],
    visibility = ["//visibility:public"],
)