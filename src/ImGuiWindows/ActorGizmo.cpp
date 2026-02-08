#include "ActorGizmo.hpp"

#include "al/Library/Action/ActorActionKeeper.h"
#include "al/Project/Action/ActionAnimCtrl.h"
#include <al/Library/Base/StringUtil.h>
#include <al/Library/Execute/ActorExecuteInfo.h>
#include <al/Library/Model/ModelCtrl.h>
#include <al/Library/Model/ModelDrawerBase.h>
#include <al/Library/Model/ModelKeeper.h>
#include <al/Library/Player/PlayerHolder.h>
#include <al/Library/Player/PlayerUtil.h>
#include <al/Library/Scene/SceneUtil.h>

#include <game/Sequence/ChangeStageInfo.h>
#include <game/Sequence/HakoniwaSequence.h>
#include <game/System/GameDataFunction.h>
#include <game/System/GameSystem.h>

#include "custom/game/Scene/StageScene.h"

#include <getHelper.h>

#include "ActorPicking.hpp"
#include "imgui.h"
#include "ImGuizmo.h"
#include "levelStuff/LoadLevel.hpp"

static al::Scene* sLastScene = nullptr;
static al::LiveActor* sSelectedActor = nullptr;
HakoniwaSequence* gameSeq;
StageScene* stageScene;
PlayerActorHakoniwa* playerHak;
bool noGetPlayer = false;
bool gActorGizmoEnabled = false;
ImGuizmo::MODE gActorGizmoMode = ImGuizmo::LOCAL;

namespace {

// =====================
// Matrix Helpers
// =====================

inline void Matrix34ToImGuizmo(const sead::Matrix34f& m, float out[16]) {
    out[0] = m.m[0][0];
    out[4] = m.m[0][1];
    out[8] = m.m[0][2];
    out[12] = m.m[0][3];
    out[1] = m.m[1][0];
    out[5] = m.m[1][1];
    out[9] = m.m[1][2];
    out[13] = m.m[1][3];
    out[2] = m.m[2][0];
    out[6] = m.m[2][1];
    out[10] = m.m[2][2];
    out[14] = m.m[2][3];
    out[3] = 0.f;
    out[7] = 0.f;
    out[11] = 0.f;
    out[15] = 1.f;
}

inline void ImGuizmoToMatrix34(const float in[16], sead::Matrix34f& out) {
    out.m[0][0] = in[0];
    out.m[0][1] = in[4];
    out.m[0][2] = in[8];
    out.m[0][3] = in[12];
    out.m[1][0] = in[1];
    out.m[1][1] = in[5];
    out.m[1][2] = in[9];
    out.m[1][3] = in[13];
    out.m[2][0] = in[2];
    out.m[2][1] = in[6];
    out.m[2][2] = in[10];
    out.m[2][3] = in[14];
}

inline void ProjectionToImGuizmo(const sead::Projection& proj, float out[16]) {
    const auto& m = proj.getProjectionMatrix();
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            out[j * 4 + i] = m.m[i][j];
}

}  // namespace

// =====================
// Public API
// =====================

namespace actor_gizmo {

void ResetForScene(al::Scene* scene) {
    if (scene != sLastScene) {
        sSelectedActor = nullptr;  // Crash-Fix
        sLastScene = scene;
    }
}

al::LiveActor* GetSelectedActor() {
    return sSelectedActor;
}

void DrawActorBrowser(al::Scene* scene) {
    if (!scene)
        return;
    toolbar();
    LoadLevel::updateAllActorMods();
    LoadLevel::debugModifiedShit();

    ImGui::Begin("Actors");

    // === Search State ===
    static char searchBuf[128] = "";
    static bool caseInsensitive = true;

    // === Ctrl+F → Focus Search ===
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F)) {
            ImGui::SetKeyboardFocusHere();
        }
    }

    // === Search Bar + Toggle ===
    ImGui::InputTextWithHint("##ActorSearch", "Search actor...", searchBuf, sizeof(searchBuf));
    ImGui::SameLine();
    ImGui::Checkbox("Aa", &caseInsensitive);
    ImGui::Separator();

    auto* group = scene->mLiveActorKit->getLiveActorGroupAllActors();
    if (group) {
        for (int i = 0; i < group->getActorCount(); i++) {
            auto* actor = group->getActor(i);
            if (!actor)
                continue;

            const char* name = actor->mName;
            if (actor->getModelKeeper())
                name = actor->getModelKeeper()->getName();

            // === Filter ===
            if (searchBuf[0] != '\0') {
                bool match = false;

                if (caseInsensitive)
                    match = ImGuiHelpers::ImStristrLocal(name, searchBuf) != nullptr;  // for whatever reason ImGui's own function isn't working here
                else
                    match = strstr(name, searchBuf) != nullptr;

                if (!match)
                    continue;
            }

            ImGui::PushID(actor);
            if (ImGui::Selectable(name, sSelectedActor == actor))
                sSelectedActor = actor;
            ImGui::PopID();
        }
    }

    ImGui::End();
}

void DrawSelectedActorGizmo(sead::Camera* camera, sead::Projection* projection) {
    if (!camera || !projection || !sSelectedActor || !gActorGizmoEnabled)
        return;

    al::Scene* scene = helpers::tryGetScene();
    // actor_picking::Update(scene, camera);

    // al::LiveActor* hovered = actor_picking::getHoveredActor(); // Unused for now, will be used for hover highlights later

    if (actor_picking::getSelectedActor())
        sSelectedActor = actor_picking::getSelectedActor();

    if (!sSelectedActor || !sSelectedActor->mPoseKeeper)
        return;

    auto* pose = sSelectedActor->mPoseKeeper;
    auto* modelkeeper = sSelectedActor->getModelKeeper();

    sead::Matrix34f m;
    sead::Vector3f rotRad{sead::Mathf::deg2rad(pose->getRotate().x), sead::Mathf::deg2rad(pose->getRotate().y), sead::Mathf::deg2rad(pose->getRotate().z)};

    m.makeR(rotRad);
    m.setTranslation(pose->mTrans);

    float model[16], view[16], proj[16];
    Matrix34ToImGuizmo(m, model);
    Matrix34ToImGuizmo(camera->getMatrix(), view);
    ProjectionToImGuizmo(*projection, proj);

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
    ImGuizmo::SetGizmoSizeClipSpace(0.25f);

    if (ImGuizmo::Manipulate(view, proj, ImGuizmo::TRANSLATE | ImGuizmo::ROTATE, gActorGizmoMode, model)) {
        sead::Matrix34f out;
        ImGuizmoToMatrix34(model, out);

        pose->mTrans = out.getTranslation();

        sead::Vector3f rot;
        out.getRotation(rot);

        if (pose->getRotatePtr()) {
            auto* r = pose->getRotatePtr();
            r->x = sead::Mathf::rad2deg(rot.x);
            r->y = sead::Mathf::rad2deg(rot.y);
            r->z = sead::Mathf::rad2deg(rot.z);
        }
        for (s32 i = 0; i < sSelectedActor->getExecuteInfo()->getDrawerCount(); i++) {
            auto* drawer = sSelectedActor->getExecuteInfo()->getDrawer(i);
            if (drawer) {
                drawer->updateModel(modelkeeper->getModelCtrl());
            }
        }
        LoadLevel::onActorModified(sSelectedActor);
        sSelectedActor->updateCollider();
        modelkeeper->update();
        modelkeeper->updateLast();
        sSelectedActor->control();
        sSelectedActor->movement();
        sSelectedActor->calcAnim();
        sSelectedActor->draw();
        sSelectedActor->kill();
        sSelectedActor->makeActorDead();
        sSelectedActor->makeActorAlive();
        modelkeeper->hide();
        modelkeeper->show();
        sSelectedActor->appear();
        sSelectedActor->initAfterPlacement();
    }

    ImGui::Begin("Actor Transform");
    ImGui::Text("Pos: %.2f %.2f %.2f", pose->mTrans.x, pose->mTrans.y, pose->mTrans.z);
    ImGui::Text("Rot: %.1f %.1f %.1f", pose->getRotate().x, pose->getRotate().y, pose->getRotate().z);
    ImGui::Text("Scale: %.2f %.2f %.2f", pose->getScale().x, pose->getScale().y, pose->getScale().z);

    if (ImGui::TreeNode("Actor Pose")) {
        ImGuiHelpers::Vector3Drag("Trans", "Pose Keeper Translation", &pose->mTrans, 50.f, 0.f);
        ImGuiHelpers::Vector3Drag("Scale", "Pose Keeper Scale", pose->getScalePtr(), 0.05f, 0.f);
        ImGuiHelpers::Vector3Drag("Velocity", "Pose Keeper Velocity", pose->getVelocityPtr(), 1.f, 0.f);
        ImGuiHelpers::Vector3Slide("Front", "Pose Keeper Front", pose->getFrontPtr(), 1.f, true);
        ImGuiHelpers::Vector3Slide("Up", "Pose Keeper Up", pose->getUpPtr(), 1.f, true);
        ImGuiHelpers::Vector3Slide("Gravity", "Pose Keeper Gravity", pose->getGravityPtr(), 1.f, true);
        ImGuiHelpers::Vector3Drag("Euler", "Pose Keeper Rotation", pose->getRotatePtr(), 1.f, 360.f);
        ImGuiHelpers::Quat("Pose Keeper Quaternion", pose->getQuatPtr());
        ImGui::TreePop();
    }
    if (!sSelectedActor->mFlags)
        return;

    if (ImGui::TreeNode("Flags")) {
        for (int i = 0; i < 12; i++) {
            ImGui::Checkbox(flagNames[i], (bool*)((uintptr_t)sSelectedActor->mFlags + i));
        }

        ImGui::TreePop();
    }

    ImGui::Text("Stage: %s", gameSeq ? GameDataFunction::getCurrentStageName(gameSeq->mGameDataHolderAccessor) : NULL);

#pragma region Nerve stuff

    gameSeq = helpers::tryGetHakoniwaSequence();
    stageScene = helpers::tryGetStageScene(gameSeq);

    if (stageScene) {
        if (strcmp(GameDataFunction::getCurrentStageName(gameSeq->mGameDataHolderAccessor), "ClashWorldHomeStage") == 0)
            noGetPlayer = false;
    }
    if (noGetPlayer) {
        playerHak = nullptr;
    } else {
        playerHak = helpers::tryGetPlayerActorHakoniwa(gameSeq);
    }

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Demagnle Stuff")) {
#pragma region Selected Object Nerve
        if (sSelectedActor) {
            int status;
            uintptr_t NrvAddr = 0;
            al::NerveKeeper* nerveKeeper = sSelectedActor->getNerveKeeper();
            char* actorName = nullptr;
            char* nerveName = nullptr;
            char* stateName = nullptr;
            int prefixLen = 0;

            actorName = abi::__cxa_demangle(typeid(*sSelectedActor).name(), nullptr, nullptr, &status);

            if (nerveKeeper) {
                const al::Nerve* currentNerve = nerveKeeper->getCurrentNerve();
                if (currentNerve) {
                    NrvAddr = (uintptr_t)currentNerve;
                    nerveName = abi::__cxa_demangle(typeid(*currentNerve).name(), nullptr, nullptr, &status);
                    prefixLen = nerveName[0] == '(' ? strlen("(anonymous namespace)::") : 0;
                }
                if (sSelectedActor->getNerveKeeper()->mStateCtrl) {
                    al::NerveStateCtrl::State* state = sSelectedActor->getNerveKeeper()->mStateCtrl->mCurrentState;
                    if (state)
                        stateName = abi::__cxa_demangle(typeid(*state->state).name(), nullptr, nullptr, &status);
                }
            }

            ImGui::Text("Selected Actor: %s", actorName);
            ImGui::Text("SelectedActorNrv: %s", nerveName + prefixLen);
            ImGui::Text("SelectedActorState: %s", stateName);
            ImGui::Text("SelectedActorNrvAddr: %lx", NrvAddr);
            ImGui::Text("SelectedActorNrvOffset: %lx", NrvAddr - hk::ro::getMainModule()->range().start());
            ImGui::Separator();

            if (actorName)
                free(actorName);
            if (nerveName)
                free(nerveName);
            if (stateName)
                free(stateName);
        }
#pragma endregion
    }
#pragma endregion

    ImGui::End();
}

void toolbar() {
    ImGui::Begin("Actor Gizmo Toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
    ImGui::Checkbox("Enable Actor Gizmo", &gActorGizmoEnabled);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Toggle the Actor Gizmo tool.\nWhen enabled, you can manipulate the selected actor's transform in the viewport.");
    }
    if (ImGui::Combo("##GizmoMode", (int*)&gActorGizmoMode, "Local\0World\0")) {
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Change the gizmo mode between Local and World space.");
    }
    ImGui::End();
}

}  // namespace actor_gizmo
