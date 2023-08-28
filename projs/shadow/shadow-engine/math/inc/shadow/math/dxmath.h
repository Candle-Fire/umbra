#pragma once

#include <algorithm>
#include <utility>

#ifdef WIN32
#include <intrin.h>
#endif

#ifndef WIN32
#include "sal.h"

#endif

#if !defined(_MSC_VER)
#undef __cpuid
#define __cpuid(X, I0, I1, I2, I3) __cpuid(&I0, X);
#endif

#include <DirectXMath.h>