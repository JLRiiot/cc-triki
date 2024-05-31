#pragma once

#include <vector>
#include <string>


#ifdef _WIN32
  #define TRIQUI_EXPORT __declspec(dllexport)
#else
  #define TRIQUI_EXPORT
#endif

TRIQUI_EXPORT void triqui();
TRIQUI_EXPORT void triqui_print_vector(const std::vector<std::string> &strings);
