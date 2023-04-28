#pragma once

#if defined(_WIN32)
#  if defined(EXPORTING_SH_ENGINE)
#    define API __declspec(dllexport)
#  else
#    define API __declspec(dllimport)
#  endif
#else // non windows
#  define SH_EXPORT
#endif

#if defined(_WIN32)
#  if defined(EXPORTING_SH_ENGINE)
#    define API_NEW gnu::dllexport
#  else
#    define API_NEW gnu::dllimport
#  endif
#else // non windows
#  define SH_EXPORT
#endif

