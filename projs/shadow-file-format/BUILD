load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
    name = "shadow-file-format",
    srcs = glob(["src/**/*.cpp", "src/**/*.h"]),
    hdrs = glob(["src/**/*.h"]),
    strip_include_prefix = "src/",
    includes = [],
    copts = [
        "/std:c++17"
    ],
    deps = [
        "//projs/shadow-utility"
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "test",
    srcs = glob(["test/**/*.cpp", "test/**/*.h"]),
    includes = [],
    defines = [ "CATCH_CONFIG_MAIN" ],
    copts = [
        "/std:c++17"
    ],
    deps = [
        "//projs/shadow-file-format",
        "@catch2"
        ],
)