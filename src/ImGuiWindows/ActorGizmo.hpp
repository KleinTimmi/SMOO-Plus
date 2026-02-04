#pragma once

#include "hk/ro/RoUtil.h"

#include "al/Library/Action/ActorActionKeeper.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "al/Library/LiveActor/LiveActor.h"
#include "al/Library/LiveActor/LiveActorGroup.h"
#include "al/Library/LiveActor/LiveActorKit.h"
#include "al/Library/Model/ModelKeeper.h"
#include "al/Library/Nerve/Nerve.h"
#include "al/Library/Nerve/NerveKeeper.h"
#include "al/Library/Nerve/NerveStateCtrl.h"
#include "al/Library/Scene/Scene.h"

#include "sead/gfx/seadCamera.h"
#include "sead/gfx/seadProjection.h"
#include "sead/math/seadMatrix.h"

#include <cxxabi.h>

#include "imgui_str.hpp"

namespace actor_gizmo {

// Lifecycle
void ResetForScene(al::Scene* scene);

// UI
void DrawActorBrowser(al::Scene* scene);
void DrawSelectedActorGizmo(sead::Camera* camera, sead::Projection* projection);

// Utils
al::LiveActor* GetSelectedActor();

__attribute__((used)) static const char* flagNames[] = {"Dead",       "Clipped",   "Cannot Clip",    "Draw Clipped", "Calc Anim On",   "Model Visible",
                                                        "No Collide", "Unknown 8", "Valid Mat Code", "Area Target",  "Move FX Sensor", "Unknown 12"};

}  // namespace actor_gizmo
