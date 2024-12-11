/*
 * Copyright (c) 2024 Le Juez Victor
 * 
 * This software is provided "as-is", without any express or implied warranty. In no event 
 * will the authors be held liable for any damages arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose, including commercial 
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 * 
 *   1. The origin of this software must not be misrepresented; you must not claim that you 
 *   wrote the original software. If you use this software in a product, an acknowledgment 
 *   in the product documentation would be appreciated but is not required.
 * 
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 * 
 *   3. This notice may not be removed or altered from any source distribution.
 */

#ifndef R3D_MODEL_HPP
#define R3D_MODEL_HPP

#include "r3d.h"

#include <raylib.h>
#include <raymath.h>

#include <unordered_map>
#include <utility>
#include <vector>
#include <string>
#include <span>

// NOTE: The implementation is in 'src/model.cpp'

namespace r3d {

struct Model
{
    struct Animation
    {
        int boneCount;              // Number of bones
        int frameCount;             // Number of animation frames
        ::BoneInfo *bones;          // Bones information (skeleton)
        ::Transform **framePoses;   // Poses array by frame

        Animation(const ::ModelAnimation& anim)
            : boneCount(anim.boneCount)
            , frameCount(anim.frameCount)
            , bones(anim.bones)
            , framePoses(anim.framePoses)
        { }

        Animation(const Animation&) = delete;
        Animation& operator=(const Animation&) = delete;

        Animation(Animation&& other) noexcept
            : boneCount(other.boneCount)
            , frameCount(other.frameCount)
            , bones(std::exchange(other.bones, nullptr))
            , framePoses(std::exchange(other.framePoses, nullptr))
        { }

        Animation& operator=(Animation&& other) noexcept {
            if (this != &other) {
                boneCount = other.boneCount;
                frameCount = other.frameCount;
                bones = std::exchange(other.bones, nullptr);
                framePoses = std::exchange(other.framePoses, nullptr);
            }
            return *this;
        }

        ~Animation() {
            if (framePoses != nullptr) {
                for (int i = 0; i < frameCount; i++) {
                    RL_FREE(framePoses[i]);
                }
                RL_FREE(framePoses);
            }
            if (bones != nullptr) {
                RL_FREE(bones);
            }
        }
    };

    std::vector<R3D_Surface> surfaces;
    std::unordered_map<std::string, Animation> animations;

    std::span<::BoneInfo> bones;
    std::span<::Transform> bindPose;

    Model() = default;

    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    void updateAnimationBones(const struct Animation& anim, int frame) const;
};

} // namespace r3d

#endif // R3D_MODEL_HPP
