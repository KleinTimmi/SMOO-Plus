#include "LoadLevel.hpp"

#include "al/Library/Camera/CameraDirector.h"
#include "al/Library/Camera/CameraTicket.h"
#include "al/Library/Camera/CameraTicketHolder.h"
#include "al/Library/Camera/CameraUtil.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "al/Library/LiveActor/LiveActor.h"
#include "al/Library/LiveActor/LiveActorGroup.h"
#include "al/Library/LiveActor/LiveActorKit.h"
#include "al/Library/Resource/Resource.h"
#include "al/Library/Scene/Scene.h"
#include "al/Library/Stage/StageResourceKeeper.h"
#include "al/Library/Stage/StageResourceList.h"
#include "al/Library/Yaml/ByamlIter.h"

#include <sead/container/seadPtrArray.h>
#include <sead/heap/seadHeapMgr.h>
#include <sead/prim/seadSafeString.h>

#include "getHelper.h"
#include "imgui.h"

namespace LoadLevel {

// ======================================================
// Globals / Storage
// ======================================================

al::Scene* lastScene = nullptr;
al::Resource* gStageResource = nullptr;

static ActorModification* sActorModPtrStorage[256];
static ActorModification sActorModObjects[256];
static int sActorModIndex = 0;

sead::PtrArray<ActorModification> modificationBuffer;

bool gCameraDebugEnabled = false;
al::CameraTicket* gPreviewTicket = nullptr;

// ======================================================
// Actor Modification
// ======================================================

//should be called whenever an actor is modified, to store the modified values in a buffer for later use
void onActorModified(al::LiveActor* actor) {
    if (!actor || modificationBuffer.isFull())
        return;

    if (sActorModIndex >= 256)
        return;

    al::ActorPoseKeeperBase* pose = actor->getPoseKeeper();
    if (!pose)
        return;

    ActorModification* mod = &sActorModObjects[sActorModIndex++];
    mod->mActorName = actor->mName;
    mod->mPosition = pose->getTrans();
    mod->mRotation = pose->getQuat();
    mod->mScale = pose->getScale();

    modificationBuffer.pushBack(mod);
}

// ======================================================
// Scene / Stage Setup
// ======================================================

//should be called whenever a new scene is loaded, to reset the modification buffer and load the new stage resource
void onActorModifiedSetup() {
    al::Scene* scene = helpers::tryGetScene();
    if (scene == lastScene)
        return;

    lastScene = scene;

    // Reset buffer on scene change
    modificationBuffer.clear();
    modificationBuffer.allocBuffer(256, sead::HeapMgr::instance()->getCurrentHeap());
    sActorModIndex = 0;

    if (!scene)
        return;

    // Build StageMap name
    sead::FixedSafeString<128> stageName;
    stageName.copy(scene->mName.cstr());
    stageName.append("StageMap");

    gStageResource = loadStageResource(stageName.cstr());
}

// ======================================================
// Stage Resource
// ======================================================

//helper for Loading the stage resource, (wants stagenam + "StageMap")
al::Resource* loadStageResource(const char* stageName) {
    al::Scene* scene = helpers::tryGetScene();
    if (!scene)
        return nullptr;

    al::StageResourceKeeper* keeper = scene->getStageResourceKeeper();
    if (!keeper)
        return nullptr;

    al::StageResourceList* list = keeper->getStageResourceList(0);
    if (!list)
        return nullptr;

    int stageCount = list->getStageResourceNum();
    for (int i = 0; i < stageCount; i++) {
        al::StageInfo* info = list->getStageInfo(i);
        if (!info)
            continue;

        if (helpers::tryGetScene()->mName == stageName)
            return info->getResource();
    }

    return nullptr;
}

//ChatGPT said this is needed, idk about that 
void updateAllActorMods() {
    al::Scene* scene = helpers::tryGetScene();
    if (!scene)
        return;

    auto* group = scene->mLiveActorKit->getLiveActorGroupAllActors();
    if (!group)
        return;

    for (int i = 0; i < group->getActorCount(); i++) {
        al::LiveActor* actor = group->getActor(i);
        if (!actor)
            continue;

        onActorModified(actor);
    }
}


// TODO: Fix, this doesn't work at all
void debugModifiedShit() {
    ImGui::Begin("Modified Actor Debug");
    for (int i = 0; i < modificationBuffer.size(); i++) {
        ActorModification* mod = modificationBuffer[i];
        if (!mod)
            continue;
        ImGui::Text("Modified Actor: %s", mod->mActorName.cstr());
        ImGui::Text("Position: (%f, %f, %f)", mod->mPosition.x, mod->mPosition.y, mod->mPosition.z);
        ImGui::Text("Rotation: (%f, %f, %f, %f)", mod->mRotation.x, mod->mRotation.y, mod->mRotation.z, mod->mRotation.w);
        ImGui::Text("Scale: (%f, %f, %f)", mod->mScale.x, mod->mScale.y, mod->mScale.z);
    }
    ImGui::End();
}

// ======================================================
// Camera Param Debug Window
// ======================================================
// this was a TEST, and is not needed maybe I will modify this so its acctuallyuseful
void DrawStageCameraParamWindow() {
    al::Scene* scene = helpers::tryGetScene();
    if (!scene)
        return;

    ImGui::Begin("Stage Camera Param Debug");

    if (ImGui::Button("Start / Stop")) {
        gCameraDebugEnabled = !gCameraDebugEnabled;
    }

    if (!gCameraDebugEnabled) {
        ImGui::Text("Press Start to begin debugging");
        ImGui::End();
        return;
    }

    al::StageResourceKeeper* keeper = scene->getStageResourceKeeper();
    if (!keeper) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "No StageResourceKeeper");
        ImGui::End();
        return;
    }

    al::StageResourceList* list = keeper->getStageResourceList(0);
    if (!list) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "No StageResourceList");
        ImGui::End();
        return;
    }

    int stageCount = list->getStageResourceNum();
    ImGui::Text("StageResourceNum: %d", stageCount);

    for (int i = 0; i < stageCount; i++) {
        al::StageInfo* info = list->getStageInfo(i);
        if (!info)
            continue;

        al::Resource* res = info->getResource();
        if (!res)
            continue;

        ImGui::Separator();
        ImGui::Text("Index %d", i);
        ImGui::Text("Archive: %s", res->getArchiveName());

        const u8* camByml = res->tryGetByml("CameraParam");
        if (!camByml) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "CameraParam.byml: NOT FOUND");
            continue;
        }

        ImGui::TextColored(ImVec4(0, 1, 0, 1), "CameraParam.byml: FOUND");

        al::ByamlIter root(camByml);
        al::ByamlIter tickets;
        if (!root.tryGetIterByKey(&tickets, "Tickets"))
            continue;

        al::CameraDirector* director = scene->getCameraDirector();
        if (!director || !director->mTicketHolder)
            continue;

        al::LiveActor* cameraUser = helpers::tryGetPlayerActor();

        for (int t = 0; t < director->mTicketHolder->mNumTickets; t++) {
            al::CameraTicket* ticket = director->mTicketHolder->mTickets[t];
            if (!ticket)
                continue;

            sead::FixedSafeString<64> btn;
            btn.format("Preview##%d", t);

            if (ImGui::Button(btn.cstr()) && cameraUser) {
                if (gPreviewTicket)
                    al::endCamera(cameraUser, gPreviewTicket, -1, false);

                gPreviewTicket = ticket;
                al::startCamera(cameraUser, ticket);
            }
        }

        if (gPreviewTicket && cameraUser) {
            if (ImGui::Button("Stop Preview")) {
                al::endCamera(cameraUser, gPreviewTicket, -1, false);
                gPreviewTicket = nullptr;
            }
        }
    }

    ImGui::End();
}

}  // namespace LoadLevel
