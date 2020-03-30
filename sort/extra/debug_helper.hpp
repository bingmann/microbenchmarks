/*******************************************************************************
 * sort/debug_helper.hpp
 *
 * Helper DEFINES for timing functions and sections.
 *
 * Copyright (C) 2020 by Pit Henrich <pithenrich2d@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 ******************************************************************************/

#pragma once
#include <chrono>

#if 0
#define TIME_FUNCTION(function, comment) \
  {\
  auto t1 = std::chrono::high_resolution_clock::now();\
  function;\
  auto t2 = std::chrono::high_resolution_clock::now();\
  auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);\
  std::cout << "[DEBUG TIMER] " << comment << int_ms.count()/1000.0f << "s" << std::endl;\
  }

#define TIME_START() auto t1_x0 = std::chrono::high_resolution_clock::now();
#define TIME_RESET()      t1_x0 = std::chrono::high_resolution_clock::now();
#define TIME_PRINT(comment) \
{\
  auto t2_x0 = std::chrono::high_resolution_clock::now();\
  auto int_ms_x0 = std::chrono::duration_cast<std::chrono::milliseconds>(t2_x0 - t1_x0);\
  std::cout << "[CUSTOM TIMER] " << comment << int_ms_x0.count()/1000.0f << "s" << std::endl;\
}

#define TIME_PRINT_RESET(comment) \
{\
  auto t2_x0 = std::chrono::high_resolution_clock::now();\
  auto int_ms_x0 = std::chrono::duration_cast<std::chrono::milliseconds>(t2_x0 - t1_x0);\
  std::cout << "[CUSTOM TIMER] " << comment << int_ms_x0.count()/1000.0f << "s" << std::endl;\
  t1_x0 = std::chrono::high_resolution_clock::now();\
}
#else
#define TIME_FUNCTION(function, comment) function;
#define TIME_START()
#define TIME_RESET()
#define TIME_PRINT(comment)
#define TIME_PRINT_RESET(comment)
#endif
