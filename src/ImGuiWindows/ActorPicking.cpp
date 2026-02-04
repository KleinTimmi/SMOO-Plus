#pragma once

#include "ActorPicking.hpp"

#include "al/Library/Collision/KCollisionServer.h"
#include <al/Library/Collision/Collider.h>
#include <al/Library/LiveActor/LiveActorGroup.h>
#include <al/Library/LiveActor/LiveActorKit.h>
#include <al/Library/Scene/Scene.h>

#include <gfx/seadCamera.h>
#include <math/seadVector.h>

#include "imgui.h"
#include "ImGuizmo.h"

namespace actor_picking {

static al::LiveActor* sSelectedActor = nullptr;
static al::LiveActor* sHoveredActor = nullptr;

// ------------------------------
// Ray erstellen (Kamera → Welt)
// ------------------------------
static Ray makeRay(sead::Camera* camera) {
    Ray r;
    camera->getWorldPosByMatrix(&r.origin);
    camera->getLookVectorByMatrix(&r.dir);
    r.dir.normalize();
    return r;
}

// ------------------------------
// Raycast gegen Actor-Collider
// ------------------------------
// static bool rayHitsActorCollider(const Ray& ray, al::LiveActor* actor, float& outDist) {
//     if (!actor || !actor->mCollider)
//         return false;

//     al::SphereHitInfo hitInfo{};
//     al::SphereInterpolator interp{};
//     interp.mPrevPos = ray.origin;
//     interp.mCurPos  = ray.origin + ray.dir * 10000.f; // sehr langer Strahl
//     interp.mRadius  = 1.0f; // Punktstrahl

//     actor->mCollider->findCollidePos(nullptr, &interp, &hitInfo, 1);
//     if (hitInfo.mIsHit) {
//         outDist = (hitInfo.mPos - ray.origin).length();
//         return true;
//     }
//     return false;
// }

// ------------------------------
// Optionaler Screen-Space-Fallback
// (für Actors ohne Collider)
// ------------------------------
// static bool rayHitsActorScreenSpace(const Ray& ray, al::LiveActor* actor, float& outDist) {
//     if (!actor || !actor->mPoseKeeper)
//         return false;

//     sead::Vector3f toActor = actor->getPoseKeeper()->mTrans - ray.origin;
//     float proj = toActor.dot(ray.dir);
//     if (proj < 0.0f)
//         return false; // hinter Kamera

//     float distSq = (toActor - ray.dir * proj).lengthSquared();
//     constexpr float fallbackRadius = 120.0f; // fallback Sphere
//     if (distSq > fallbackRadius * fallbackRadius)
//         return false;

//     outDist = proj;
//     return true;
// }

// ------------------------------
// Update: Hover + Click
// ------------------------------
// void Update(al::Scene* scene, sead::Camera* camera) {
//     sHoveredActor = nullptr;
//     if (!scene || !camera)
//         return;

//     ImGuiIO& io = ImGui::GetIO();
//     if (io.WantCaptureMouse) return;          // UI blockiert Maus
//     if (ImGuizmo::IsOver() || ImGuizmo::IsUsing()) return; // Gizmo blockiert

//     Ray ray = makeRay(camera);
//     auto* group = scene->mLiveActorKit->getLiveActorGroupAllActors();
//     if (!group) return;

//     al::LiveActor* best = nullptr;
//     float bestDist = FLT_MAX;

//     for (int i = 0; i < group->getActorCount(); i++) {
//         al::LiveActor* actor = group->getActor(i);
//         if (!actor) continue;

//         float dist;
//         bool hit = false;

//         if (actor->mCollider)
//             hit = rayHitsActorCollider(ray, actor, dist);
//         else
//             hit = rayHitsActorScreenSpace(ray, actor, dist); // Fallback

//         if (hit && dist < bestDist) {
//             bestDist = dist;
//             best = actor;
//         }
//     }

//     // Hover nur, wenn Maus nicht gedrückt
//     if (!io.MouseDown[ImGuiMouseButton_Left])
//         sHoveredActor = best;

//     // Click → Auswahl
//     if (best && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
//         sSelectedActor = best;
// }

// ------------------------------
// Getter
// ------------------------------
al::LiveActor* getSelectedActor() {
    return sSelectedActor;
}
al::LiveActor* getHoveredActor() {
    return sHoveredActor;
}

}  // namespace actor_picking
