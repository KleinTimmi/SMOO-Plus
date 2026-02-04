#include "al/Library/Sequence/Sequence.h"

#include "game/Player/PlayerActorBase.h"
#include "game/System/GameDataHolderAccessor.h"
#include "al/Library/Camera/CameraUtil.h"

static bool isInGame = false;

static bool debugMode = false;

// --- ImGuizmo ---
void ImGuiZmo(sead::Camera* camera, const sead::Projection& projection);
void Matrix34ToImGuizmo(const sead::Matrix34f& m, float out[16]);
void ImGuizmoToMatrix34(const float in[16], sead::Matrix34f& out);
void projectionMatrixToImGuizmo(const sead::Projection& proj, float out[16]);

// --- Game main draw ---
void drawMain(al::Sequence* seq);

// --- Player info ---
void updatePlayerInfo(GameDataHolderAccessor holder, PlayerActorBase* playerBase, bool isYukimaru);

constexpr const char* captureNames[] = {"AnagramAlphabetCharacter",
                                        "Byugo",
                                        "Bubble",
                                        "Bull",
                                        "Car",
                                        "ElectricWire",
                                        "JugemFishing",
                                        "Statue",
                                        "Fukankun",
                                        "Yoshi",
                                        "KillerLauncherMagnum",
                                        "KuriboPossessed",
                                        "WanwanBig",  // has sub-actors
                                        "KillerLauncher",
                                        "Koopa",
                                        "Wanwan",  // has sub-actors
                                        "Pukupuku",
                                        "PukupukuSnow",
                                        "Gamane",  // has sub-actors
                                        "FireBrosPossessed",
                                        "PackunFire",
                                        "Frog",
                                        "Kakku",
                                        "Hosui",
                                        "HammerBrosPossessed",
                                        "Megane",
                                        "KaronWing",
                                        "KuriboWing",
                                        "PackunPoison",
                                        "Radicon",
                                        "Tank",
                                        "Tsukkun",
                                        "TRex",
                                        "TRexSleep",
                                        "TRexPatrol",
                                        "Imomu",
                                        "SenobiGeneratePoint"};