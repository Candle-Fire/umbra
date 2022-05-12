load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "shadow-entity",
    srcs = glob(["**/*.cpp", "**/*.h"]),
    hdrs = [],
    includes = [],
    copts = [
        #"/experimental:module",
        "/std:c++17"
    ],
    deps = [
        "//src/shadow-reflection",
        "//src/shadow-math",
        "@glm_lib//:glm"
    ],
)