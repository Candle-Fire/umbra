//
// Created by dpete on 04/09/2022.
//

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

