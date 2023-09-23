#pragma once

#ifdef SH_ENABLE_ASSERTS
    #define SH_ASSERT(x, ...) { if(!(x)) { SH_ERROR(__VA_ARGS__); __debugbreak(); } }

	#define SH_CORE_ASSERT(x, ...) { if(!(x)) { SH_CORE_ERROR(__VA_ARGS__); __debugbreak(); } }
#else
    #define SH_ASSERT(x, ...)
    #define SH_CORE_ASSERT(x, ...)
#endif
