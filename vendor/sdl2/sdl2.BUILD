load("@rules_cc//cc:defs.bzl", "cc_library")

alias(
	name = "windows",
	actual  = "@bazel_tools//src/conditions:windows"
)

alias(
	name = "macos",
	actual  = "@bazel_tools//src/conditions:darwin_x86_64"
)

alias(
	name = "linux",
	actual  = "@bazel_tools//src/conditions:linux_x86_64"
)



cc_library(
    name = "sdl2_win",
    srcs= [
        "x86_64-w64-mingw32/lib/libSDL2.a",
        "x86_64-w64-mingw32/lib/libSDL2main.a"
        ],
    hdrs = glob(["x86_64-w64-mingw32/include/**/*.h"]),
    includes = ["x86_64-w64-mingw32/include"], # Optional. 
    linkopts = [
        "-DEFAULTLIB:user32",
		"-DEFAULTLIB:gdi32",
		"-DEFAULTLIB:winmm",
		"-DEFAULTLIB:imm32",
		"-DEFAULTLIB:ole32",
		"-DEFAULTLIB:oleaut32",
		"-DEFAULTLIB:version",
		"-DEFAULTLIB:uuid",
		"-DEFAULTLIB:shell32",
		"-DEFAULTLIB:advapi32",
		"-DEFAULTLIB:hid",
		"-DEFAULTLIB:setupapi",
		"-DEFAULTLIB:opengl32",
		"-DEFAULTLIB:kernel32",
        "/ENTRY:WinMainCRTStartup"
    ],
    visibility = ["//visibility:public"],
)


alias(
	name = "sdl2",
	actual  = select({
	    "//:windows": "//:sdl2_win",
	    "//conditions:default": "//:sdl2_win",
    }),
    visibility = ["//visibility:public"],
)