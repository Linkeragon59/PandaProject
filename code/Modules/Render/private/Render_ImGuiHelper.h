#pragma once

// Define the ImGUIContext as a thread local variable, so that we can safely use several contexts from multiple threads.
struct ImGuiContext;
extern thread_local ImGuiContext* MyImGuiTLS;
#define GImGui MyImGuiTLS

#include <imgui.h>
#include <algorithm>

static inline bool operator==(const ImVec2& aVec1, const ImVec2& aVec2) { return aVec1.x == aVec2.x && aVec1.y == aVec2.y; }
static inline bool operator!=(const ImVec2& aVec1, const ImVec2& aVec2) { return !(aVec1 == aVec2); }
static inline ImVec2 operator*(const float aScalar, const ImVec2& aVec) { return ImVec2(aVec.x * aScalar, aVec.y * aScalar); }
static inline ImVec2 operator*(const ImVec2& aVec, const float aScalar) { return ImVec2(aVec.x * aScalar, aVec.y * aScalar); }
static inline ImVec2 operator/(const ImVec2& aVec, const float aScalar) { return ImVec2(aVec.x / aScalar, aVec.y / aScalar); }
static inline ImVec2 operator+(const ImVec2& aVec1, const ImVec2& aVec2) { return ImVec2(aVec1.x + aVec2.x, aVec1.y + aVec2.y); }
static inline ImVec2 operator-(const ImVec2& aVec1, const ImVec2& aVec2) { return ImVec2(aVec1.x - aVec2.x, aVec1.y - aVec2.y); }
static inline ImVec2 operator*(const ImVec2& aVec1, const ImVec2& aVec2) { return ImVec2(aVec1.x * aVec2.x, aVec1.y * aVec2.y); }
static inline ImVec2 operator/(const ImVec2& aVec1, const ImVec2& aVec2) { return ImVec2(aVec1.x / aVec2.x, aVec1.y / aVec2.y); }
static inline ImVec2& operator*=(const float aScalar, ImVec2& aVec1) { aVec1.x *= aScalar; aVec1.y *= aScalar; return aVec1; }
static inline ImVec2& operator*=(ImVec2& aVec1, const float aScalar) { aVec1.x *= aScalar; aVec1.y *= aScalar; return aVec1; }
static inline ImVec2& operator/=(ImVec2& aVec1, const float aScalar) { aVec1.x /= aScalar; aVec1.y /= aScalar; return aVec1; }
static inline ImVec2& operator+=(ImVec2& aVec1, const ImVec2& aVec2) { aVec1.x += aVec2.x; aVec1.y += aVec2.y; return aVec1; }
static inline ImVec2& operator-=(ImVec2& aVec1, const ImVec2& aVec2) { aVec1.x -= aVec2.x; aVec1.y -= aVec2.y; return aVec1; }
static inline ImVec2& operator*=(ImVec2& aVec1, const ImVec2& aVec2) { aVec1.x *= aVec2.x; aVec1.y *= aVec2.y; return aVec1; }
static inline ImVec2& operator/=(ImVec2& aVec1, const ImVec2& aVec2) { aVec1.x /= aVec2.x; aVec1.y /= aVec2.y; return aVec1; }
static inline ImVec2 Clamp(const ImVec2& aVec, const ImVec2& aMinVec, const ImVec2& aMaxVec) { return ImVec2(std::clamp(aVec.x, aMinVec.x, aMaxVec.x), std::clamp(aVec.y, aMinVec.y, aMaxVec.y)); }
