#pragma once

#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "al/Library/LiveActor/LiveActor.h"
#include "al/Library/Scene/Scene.h"
#include <al/Library/LiveActor/LiveActorGroup.h>

#include <gfx/seadCamera.h>
#include <math/seadVector.h>

namespace actor_picking {

struct Ray {
    sead::Vector3f origin;
    sead::Vector3f dir;
};

void Update(al::Scene* scene, sead::Camera* camera);

al::LiveActor* getSelectedActor();
al::LiveActor* getHoveredActor();

}  // namespace actor_picking
