#pragma once

#include <algorithm>
#include <utility>

#include <intrin.h>

#if !defined(_MSC_VER)
#undef __cpuid
#define __cpuid(X, I0, I1, I2, I3) __cpuid(&I0, X);
#endif

#include <DirectXMath.h>