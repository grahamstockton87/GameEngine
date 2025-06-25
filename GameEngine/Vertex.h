#pragma once
#include <glm.hpp>
// in Vertex.h
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    // new:
    int     BoneIDs[4] = { 0, 0, 0, 0 };
    float   Weights[4] = { 0.f, 0.f, 0.f };

    void AddBoneData(int boneID, float weight) {
        for (int i = 0; i < 4; ++i) {
            if (Weights[i] == 0.f) {
                BoneIDs[i] = boneID;
                Weights[i] = weight;
                return;
            }
        }
        // too many bones; you can either discard extras or normalize/truncate
    }
};
