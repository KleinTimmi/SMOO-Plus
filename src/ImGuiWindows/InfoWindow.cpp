#include "InfoWindow.hpp"

#include "al/Library/Nerve/Nerve.h"
#include "al/Library/Nerve/NerveKeeper.h"
#include "al/Library/Nerve/NerveStateCtrl.h"

#include <cxxabi.h>
#include <getHelper.h>

#include "imgui.h"

StageScene* sStageScene;
HakoniwaSequence* gGameSeq;
PlayerActorHakoniwa* playerHako;
bool bNoGetPlayer = false;

void showInfoWindow() {
    ImGui::Begin("Sequence Info");

    sequenceInfoWindow_Child();

    ImGui::End();
}

void nerveInfoWindow_Child() {
    gGameSeq = helpers::tryGetHakoniwaSequence();
    sStageScene = helpers::tryGetStageScene(gGameSeq);

#pragma region SequenceNerve
    if (gGameSeq) {
        int status;
        uintptr_t NrvAddr = 0;
        al::NerveKeeper* nerveKeeper = gGameSeq->getNerveKeeper();
        char* sequenceName = nullptr;
        char* nerveName = nullptr;
        char* stateName = nullptr;
        int prefixLen = 0;

        sequenceName = abi::__cxa_demangle(typeid(*gGameSeq).name(), nullptr, nullptr, &status);

        if (nerveKeeper) {
            const al::Nerve* currentNerve = nerveKeeper->getCurrentNerve();
            if (currentNerve) {
                NrvAddr = (uintptr_t)currentNerve;
                nerveName = abi::__cxa_demangle(typeid(*currentNerve).name(), nullptr, nullptr, &status);
                prefixLen = nerveName[0] == '(' ? strlen("(anonymous namespace)::") : 0;
            }
            if (gGameSeq->getNerveKeeper()->mStateCtrl) {
                al::NerveStateCtrl::State* state = gGameSeq->getNerveKeeper()->mStateCtrl->mCurrentState;
                if (state)
                    stateName = abi::__cxa_demangle(typeid(*state->state).name(), nullptr, nullptr, &status);
            }
        }

        ImGui::Text("Sequence: %s", sequenceName);
        ImGui::Text("SequenceNrv: %s", nerveName + prefixLen);
        ImGui::Text("SequenceState: %s", stateName);
        ImGui::Text("SequenceNrvAddr: %lx", NrvAddr);
        ImGui::Text("SequenceNrvOffset: %lx", NrvAddr - hk::ro::getMainModule()->range().start());
        ImGui::Separator();

        if (sequenceName)
            free(sequenceName);
        if (nerveName)
            free(nerveName);
        if (stateName)
            free(stateName);
    }
#pragma endregion
#pragma region StageSceneNerve
    if (sStageScene) {
        int status;
        uintptr_t NrvAddr = 0;
        al::NerveKeeper* nerveKeeper = sStageScene->getNerveKeeper();
        char* sceneName = nullptr;
        char* nerveName = nullptr;
        char* stateName = nullptr;
        int prefixLen = 0;

        sceneName = abi::__cxa_demangle(typeid(*sStageScene).name(), nullptr, nullptr, &status);

        if (nerveKeeper) {
            const al::Nerve* currentNerve = nerveKeeper->getCurrentNerve();
            if (currentNerve) {
                NrvAddr = (uintptr_t)currentNerve;
                nerveName = abi::__cxa_demangle(typeid(*currentNerve).name(), nullptr, nullptr, &status);
                prefixLen = nerveName[0] == '(' ? strlen("(anonymous namespace)::") : 0;
            }
            if (sStageScene->getNerveKeeper()->mStateCtrl) {
                al::NerveStateCtrl::State* state = sStageScene->getNerveKeeper()->mStateCtrl->mCurrentState;
                if (state)
                    stateName = abi::__cxa_demangle(typeid(*state->state).name(), nullptr, nullptr, &status);
            }
        }

        ImGui::Text("StageScene: %s", sceneName);
        ImGui::Text("StageSceneNrv: %s", nerveName + prefixLen);
        ImGui::Text("StageSceneState: %s", stateName);
        ImGui::Text("StageSceneNrvAddr: %lx", NrvAddr);
        ImGui::Text("StageSceneNrvOffset: %lx", NrvAddr - hk::ro::getMainModule()->range().start());
        ImGui::Separator();

        if (sceneName)
            free(sceneName);
        if (nerveName)
            free(nerveName);
        if (stateName)
            free(stateName);
    }
#pragma endregion
#pragma region PlayerNerve
    if (playerHako) {
        int status;
        uintptr_t NrvAddr = 0;
        al::NerveKeeper* nerveKeeper = playerHako->getNerveKeeper();
        char* actorName = nullptr;
        char* nerveName = nullptr;
        char* stateName = nullptr;
        int prefixLen = 0;

        actorName = abi::__cxa_demangle(typeid(*playerHako).name(), nullptr, nullptr, &status);

        if (nerveKeeper) {
            const al::Nerve* currentNerve = nerveKeeper->getCurrentNerve();
            if (currentNerve) {
                NrvAddr = (uintptr_t)currentNerve;
                nerveName = abi::__cxa_demangle(typeid(*currentNerve).name(), nullptr, nullptr, &status);
                prefixLen = nerveName[0] == '(' ? strlen("(anonymous namespace)::") : 0;
            }
            if (playerHako->getNerveKeeper()->mStateCtrl) {
                al::NerveStateCtrl::State* state = playerHako->getNerveKeeper()->mStateCtrl->mCurrentState;
                if (state)
                    stateName = abi::__cxa_demangle(typeid(*state->state).name(), nullptr, nullptr, &status);
            }
        }

        ImGui::Text("Player: %s", actorName);
        ImGui::Text("PlayerNrv: %s", nerveName + prefixLen);
        ImGui::Text("PlayerState: %s", stateName);
        ImGui::Text("PlayerNrvAddr: %lx", NrvAddr);
        ImGui::Text("PlayerNrvOffset: %lx", NrvAddr - hk::ro::getMainModule()->range().start());
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