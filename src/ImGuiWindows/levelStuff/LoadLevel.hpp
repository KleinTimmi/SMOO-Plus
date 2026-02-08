#include "al/Library/LiveActor/LiveActor.h"
#include "al/Library/Resource/Resource.h"
#include "al/Library/Scene/Scene.h"
namespace LoadLevel {

struct ActorModification {
    sead::SafeString mActorName;
    sead::Vector3f mPosition;
    sead::Quatf mRotation;
    sead::Vector3f mScale;
};

void DrawStageCameraParamWindow();
void debugModifiedShit();
void updateAllActorMods();

void onActorModified(al::LiveActor* actor);
void loadStageByml(al::Resource* res);
al::Resource* loadStageResource(const char* stageName);
}  // namespace LoadLevel