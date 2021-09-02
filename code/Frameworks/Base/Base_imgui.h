#pragma once

// Define the ImGUIContext as a thread local variable, so that we can safely use several contexts from multiple threads.
struct ImGuiContext;
extern thread_local ImGuiContext* MyImGuiTLS;
#define GImGui MyImGuiTLS

#include <imgui.h>
