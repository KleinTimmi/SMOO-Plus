#pragma once

#include "sead/math/seadQuat.h"
#include "sead/math/seadVector.h"

#include "imgui.h"

namespace ImGuiHelpers {

// Case-insensitive strstr (ASCII)
const char* ImStristrLocal(const char* haystack, const char* needle);

void Vector3Drag(const char* prefixName, const char* tooltip, sead::Vector3f* vec, float speed = 0.1f, float limit = 0.0f);

void Vector3Slide(const char* prefixName, const char* tooltip, sead::Vector3f* vec, float limit = 1.0f, bool isNormalize = false);

void Quat(const char* tooltip, sead::Quatf* quat);

}  // namespace ImGuiHelpers
