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

#include "r3d.h"

#include "./model.hpp"

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cfloat>

/* Public API */

R3D_Model R3D_LoadModel(const char* fileName)
{
    Model rlModel = LoadModel(fileName);
    r3d::Model *r3dModel = new r3d::Model();

    r3dModel->surfaces.reserve(rlModel.meshCount);

    for (int i = 0; i < rlModel.meshCount; i++) {
        R3D_Surface& surface = r3dModel->surfaces.emplace_back(
            R3D_CreateMaterial(R3D_GetDefaultMaterialConfig()),
            rlModel.meshes[i]
        );

        const ::Material& material = rlModel.materials[rlModel.meshMaterial[i]];

        surface.material.albedo.color = material.maps[MATERIAL_MAP_ALBEDO].color;
        surface.material.emission.color = material.maps[MATERIAL_MAP_EMISSION].color;
        surface.material.ao.lightAffect = material.maps[MATERIAL_MAP_OCCLUSION].value;
        surface.material.metalness.factor = material.maps[MATERIAL_MAP_METALNESS].value;
        surface.material.roughness.factor = material.maps[MATERIAL_MAP_ROUGHNESS].value;

        if (material.maps[MATERIAL_MAP_ALBEDO].texture.id > 0) {
            surface.material.albedo.texture = material.maps[MATERIAL_MAP_ALBEDO].texture;
        }

        if (material.maps[MATERIAL_MAP_EMISSION].texture.id > 0) {
            surface.material.emission.texture = material.maps[MATERIAL_MAP_EMISSION].texture;
        }

        if (material.maps[MATERIAL_MAP_OCCLUSION].texture.id > 0) {
            surface.material.ao.texture = material.maps[MATERIAL_MAP_OCCLUSION].texture;
        }

        if (material.maps[MATERIAL_MAP_METALNESS].texture.id > 0) {
            surface.material.metalness.texture = material.maps[MATERIAL_MAP_METALNESS].texture;
        }

        if (material.maps[MATERIAL_MAP_ROUGHNESS].texture.id > 0) {
            surface.material.roughness.texture = material.maps[MATERIAL_MAP_ROUGHNESS].texture;
        }
    }

    r3dModel->bones = std::span<::BoneInfo>(rlModel.bones, rlModel.boneCount);
    r3dModel->bindPose = std::span<::Transform>(rlModel.bindPose, rlModel.boneCount);

    R3D_Model model = {
        .transform = R3D_CreateTransformIdentity(nullptr),
        .aabb = {},
        .shadow = R3D_CAST_ON,
        .billboard = R3D_BILLBOARD_DISABLED,
        .internal = r3dModel,
    };

    R3D_UpdateModelAABB(&model, 0.0f);

    return model;
}

R3D_Model R3D_LoadModelFromMesh(Mesh mesh)
{
    r3d::Model *r3dModel = new r3d::Model();

    r3dModel->surfaces.emplace_back(
        R3D_CreateMaterial(R3D_GetDefaultMaterialConfig()),
        mesh
    );

    R3D_Model model = {
        .transform = R3D_CreateTransformIdentity(nullptr),
        .aabb = {},
        .shadow = R3D_CAST_ON,
        .billboard = R3D_BILLBOARD_DISABLED,
        .internal = r3dModel,
    };

    R3D_UpdateModelAABB(&model, 0.0f);

    return model;
}

void R3D_UnloadModel(R3D_Model* model)
{
    if (model->internal != nullptr) {
        delete reinterpret_cast<r3d::Model*>(model->internal);
        model->internal = nullptr;
    }
}

int R3D_GetSurfaceCount(const R3D_Model* model)
{
    return static_cast<r3d::Model*>(model->internal)->surfaces.size();
}

R3D_Surface* R3D_GetSurface(R3D_Model* model, int surfaceIndex)
{
    return &static_cast<r3d::Model*>(model->internal)->surfaces[surfaceIndex];
}

Mesh* R3D_GetMesh(R3D_Model* model, int surfaceIndex)
{
    return &static_cast<r3d::Model*>(model->internal)->surfaces[surfaceIndex].mesh;
}

R3D_Material* R3D_GetMaterial(R3D_Model* model, int surfaceIndex)
{
    return &static_cast<r3d::Model*>(model->internal)->surfaces[surfaceIndex].material;
}

void R3D_SetMaterial(R3D_Model* model, int surfaceIndex, const R3D_Material* material)
{
    static_cast<r3d::Model*>(model->internal)->surfaces[surfaceIndex].material = *material;
}

R3D_MaterialConfig R3D_GetMaterialConfig(R3D_Model* model, int surfaceIndex)
{
    return static_cast<r3d::Model*>(model->internal)->surfaces[surfaceIndex].material.config;
}

void R3D_SetMaterialConfig(R3D_Model* model, int surfaceIndex, R3D_MaterialConfig config)
{
    static_cast<r3d::Model*>(model->internal)->surfaces[surfaceIndex].material.config = config;
}

void R3D_SetMapAlbedo(R3D_Model* model, int surfaceIndex, const Texture2D* texture, Color color)
{
    auto& material = static_cast<r3d::Model*>(model->internal)->surfaces[surfaceIndex].material;
    if (texture) material.albedo.texture = *texture;
    material.albedo.color = color;
}

void R3D_SetMapMetalness(R3D_Model* model, int surfaceIndex, const Texture2D* texture, float factor)
{
    auto& material = static_cast<r3d::Model*>(model->internal)->surfaces[surfaceIndex].material;
    if (texture) material.metalness.texture = *texture;
    material.metalness.factor = factor;
}

void R3D_SetMapRoughness(R3D_Model* model, int surfaceIndex, const Texture2D* texture, float factor)
{
    auto& material = static_cast<r3d::Model*>(model->internal)->surfaces[surfaceIndex].material;
    if (texture) material.roughness.texture = *texture;
    material.roughness.factor = factor;
}

void R3D_SetMapEmission(R3D_Model* model, int surfaceIndex, const Texture2D* texture, float energy, Color color)
{
    auto& material = static_cast<r3d::Model*>(model->internal)->surfaces[surfaceIndex].material;
    if (texture) material.emission.texture = *texture;
    material.emission.energy = energy;
    material.emission.color = color;
}

void R3D_SetMapNormal(R3D_Model* model, int surfaceIndex, const Texture2D* texture)
{
    auto& material = static_cast<r3d::Model*>(model->internal)->surfaces[surfaceIndex].material;
    if (texture) material.normal.texture = *texture;
}

void R3D_SetMapAO(R3D_Model* model, int surfaceIndex, const Texture2D* texture, float lightAffect)
{
    auto& material = static_cast<r3d::Model*>(model->internal)->surfaces[surfaceIndex].material;
    if (texture) material.ao.texture = *texture;
    material.ao.lightAffect = lightAffect;
}

void R3D_LoadModelAnimations(R3D_Model* model, const char* fileName)
{
    r3d::Model *r3dModel = static_cast<r3d::Model*>(model->internal);

    int animCount = 0;
    ::ModelAnimation *anims = LoadModelAnimations(fileName, &animCount);

    for (int i = 0; i < animCount; i++) {
        const ::ModelAnimation& anim = anims[i];
        r3dModel->animations.insert_or_assign(anim.name, anim);
    }
}

int R3D_GetModelAnimationCount(const R3D_Model* model)
{
    r3d::Model *r3dModel = static_cast<r3d::Model*>(model->internal);
    return r3dModel->animations.size();
}

char** R3D_GetModelAnimationNames(const R3D_Model* model, int* count)
{
    r3d::Model *r3dModel = static_cast<r3d::Model*>(model->internal);

    char** animNames = static_cast<char**>(RL_MALLOC(r3dModel->animations.size() * sizeof(char*)));

    int animount = 0;
    for (auto& [name, anim] : r3dModel->animations) {
        animNames[animount++] = static_cast<char*>(RL_CALLOC(name.size() + 1, sizeof(char)));
        std::strncpy(animNames[animount], name.data(), name.size());
    }

    return animNames;
}

void R3D_UnloadAnimationNames(char** names, int count)
{
    for (int i = 0; i < count; i++) {
        RL_FREE(names[i]);
    }
    RL_FREE(names);
}

void R3D_UpdateModelAnimation(R3D_Model* model, const char* name, int frame)
{
    r3d::Model *r3dModel = static_cast<r3d::Model*>(model->internal);
    const r3d::Model::Animation& anim = r3dModel->animations.at(name);

    r3dModel->updateAnimationBones(anim, frame);

    for (auto& surface : r3dModel->surfaces) {
        Mesh mesh = surface.mesh;
        Vector3 animVertex = { 0 };
        Vector3 animNormal = { 0 };
        int boneId = 0;
        int boneCounter = 0;
        float boneWeight = 0.0f;
        bool updated = false;           // Flag to check when anim vertex information is updated
        const int vValues = 3 * mesh.vertexCount;

        for (int vCounter = 0; vCounter < vValues; vCounter += 3) {
            mesh.animVertices[vCounter] = 0;
            mesh.animVertices[vCounter + 1] = 0;
            mesh.animVertices[vCounter + 2] = 0;

            if (mesh.animNormals != nullptr) {
                mesh.animNormals[vCounter] = 0;
                mesh.animNormals[vCounter + 1] = 0;
                mesh.animNormals[vCounter + 2] = 0;
            }

            // Iterates over 4 bones per vertex
            for (int j = 0; j < 4; j++, boneCounter++) {
                boneWeight = mesh.boneWeights[boneCounter];
                boneId = mesh.boneIds[boneCounter];

                // Early stop when no transformation will be applied
                if (boneWeight == 0.0f) continue;
                animVertex = (Vector3){ mesh.vertices[vCounter], mesh.vertices[vCounter + 1], mesh.vertices[vCounter + 2] };
                animVertex = Vector3Transform(animVertex, surface.mesh.boneMatrices[boneId]);
                mesh.animVertices[vCounter] += animVertex.x * boneWeight;
                mesh.animVertices[vCounter+1] += animVertex.y * boneWeight;
                mesh.animVertices[vCounter+2] += animVertex.z * boneWeight;
                updated = true;

                // Normals processing
                // NOTE: We use meshes.baseNormals (default normal) to calculate meshes.normals (animated normals)
                if (mesh.normals != nullptr) {
                    animNormal = (Vector3){ mesh.normals[vCounter], mesh.normals[vCounter + 1], mesh.normals[vCounter + 2] };
                    animNormal = Vector3Transform(animNormal, MatrixTranspose(MatrixInvert(surface.mesh.boneMatrices[boneId])));
                    mesh.animNormals[vCounter] += animNormal.x * boneWeight;
                    mesh.animNormals[vCounter + 1] += animNormal.y * boneWeight;
                    mesh.animNormals[vCounter + 2] += animNormal.z * boneWeight;
                }
            }
        }

        if (updated) {
            rlUpdateVertexBuffer(mesh.vboId[0], mesh.animVertices, 3 * mesh.vertexCount * sizeof(float), 0); // Update vertex position
            rlUpdateVertexBuffer(mesh.vboId[2], mesh.animNormals, 3 * mesh.vertexCount * sizeof(float), 0);  // Update vertex normals
        }
    }
}

void R3D_UpdateModelAABB(R3D_Model* model, float extraMargin)
{
    const std::vector<R3D_Surface>& surfaces = static_cast<r3d::Model*>(model->internal)->surfaces;

    if (surfaces.empty()) return;

    Vector3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
    Vector3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (const auto& surface : surfaces) {
        const Mesh& mesh = surface.mesh;

        for (int i = 0; i < mesh.vertexCount; ++i) {
            Vector3 vertex = { 
                mesh.vertices[i * 3], 
                mesh.vertices[i * 3 + 1], 
                mesh.vertices[i * 3 + 2] 
            };

            min.x = std::min(min.x, vertex.x);
            min.y = std::min(min.y, vertex.y);
            min.z = std::min(min.z, vertex.z);

            max.x = std::max(max.x, vertex.x);
            max.y = std::max(max.y, vertex.y);
            max.z = std::max(max.z, vertex.z);
        }
    }

    model->aabb.min = min;
    model->aabb.max = max;

    if (extraMargin != 0.0f) {
        Vector3 margin = { extraMargin, extraMargin, extraMargin };
        model->aabb.min = Vector3Subtract(model->aabb.min, margin);
        model->aabb.max = Vector3Add(model->aabb.max, margin);
    }
}

void R3D_GenTangents(R3D_Model* model)
{
    auto r3dModel = static_cast<r3d::Model*>(model->internal);
    for (auto& surface : r3dModel->surfaces) {
        GenMeshTangents(&surface.mesh);
    }
}
