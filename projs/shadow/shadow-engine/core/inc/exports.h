#pragma once

#if defined(_WIN32)
#  if defined(EXPORTING_SH_ENGINE)
#    define API __declspec(dllexport)
#  else
#    define API __declspec(dllimport)
#  endif
#else // non windows
#  define API
#endif

#if defined(_WIN32)
#    define EXPORT __declspec(dllexport)
#else // non windows
#  define EXPORT
#endif
