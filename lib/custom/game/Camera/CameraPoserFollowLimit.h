#pragma once

#include <al/Library/Camera/CameraPoser.h>

class CameraPoserFollowLimit : public al::CameraPoser {
public:
    explicit CameraPoserFollowLimit(const char* name);

private:
    u8 _padFollowLimit[0x2A0 - sizeof(al::CameraPoser)];
};

static_assert(sizeof(CameraPoserFollowLimit) == 0x2A0, "Size mismatch");
