#pragma once

#include <stdint.h>
#include <limits.h>
#include <vector>
#include <array>
#include <string>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint32_t uint;
typedef uint64_t uint64;

#ifdef NDEBUG
#define DEBUG_BUILD 0
#else
#define DEBUG_BUILD 1
#endif

#if defined(_WINDOWS)
#define WINDOWS_BUILD 1
#define LINUX_BUILD 0
#elif defined(__linux__)
#define WINDOWS_BUILD 0
#define LINUX_BUILD 1
#endif
