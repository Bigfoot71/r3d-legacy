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
#include <cassert>
#include <vector>
#include <string>
#include <span>

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

/* Public implementation */


inline Model::~Model()
{
    for (const auto& surface : surfaces) {
        UnloadMesh(surface.mesh);
    }

    if (!bones.empty()) {
        std::free(bones.data());
    }

    if (!bindPose.empty()) {
        std::free(bindPose.data());
    }
}

inline void Model::updateAnimationBones(const Animation& anim, int frame) const
{
    if ((anim.frameCount > 0) && (anim.bones != nullptr) && (anim.framePoses != nullptr)) {
        if (frame >= anim.frameCount) {
            frame = frame % anim.frameCount;
        }

        for (auto& surface : surfaces) {
            if (surface.mesh.boneMatrices) {
                assert(surface.mesh.boneCount == anim.boneCount);

                for (int boneId = 0; boneId < surface.mesh.boneCount; boneId++) {
                    Vector3 inTranslation = bindPose[boneId].translation;
                    Quaternion inRotation = bindPose[boneId].rotation;
                    Vector3 inScale = bindPose[boneId].scale;

                    Vector3 outTranslation = anim.framePoses[frame][boneId].translation;
                    Quaternion outRotation = anim.framePoses[frame][boneId].rotation;
                    Vector3 outScale = anim.framePoses[frame][boneId].scale;

                    Quaternion invRotation = QuaternionInvert(inRotation);
                    Vector3 invTranslation = Vector3RotateByQuaternion(Vector3Negate(inTranslation), invRotation);
                    Vector3 invScale = Vector3Divide((Vector3){ 1.0f, 1.0f, 1.0f }, inScale);

                    Vector3 boneTranslation = Vector3Add(Vector3RotateByQuaternion(
                        Vector3Multiply(outScale, invTranslation), outRotation), outTranslation);
                    Quaternion boneRotation = QuaternionMultiply(outRotation, invRotation);
                    Vector3 boneScale = Vector3Multiply(outScale, invScale);

                    Matrix boneMatrix = MatrixMultiply(MatrixMultiply(
                        QuaternionToMatrix(boneRotation),
                        MatrixTranslate(boneTranslation.x, boneTranslation.y, boneTranslation.z)),
                        MatrixScale(boneScale.x, boneScale.y, boneScale.z));

                    surface.mesh.boneMatrices[boneId] = boneMatrix;
                }
            }
        }
    }
}

} // namespace r3d

#endif // R3D_MODEL_HPP
