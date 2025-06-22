#pragma once

#include <assimp/scene.h>
#include <glm.hpp>
#include <gtc/quaternion.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <gtc/type_ptr.hpp> // for glm::make_mat4

struct KeyPosition {
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation {
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale {
    glm::vec3 scale;
    float timeStamp;
};

class Bone {
public:
    Bone(const std::string& name, int id, const aiNodeAnim* channel);

    void Update(float animationTime, glm::mat4& outTransform);

    std::string name;
    int id;

private:
    std::vector<KeyPosition> positions;
    std::vector<KeyRotation> rotations;
    std::vector<KeyScale> scales;

    glm::mat4 localTransform;

    int GetPositionIndex(float time);
    int GetRotationIndex(float time);
    int GetScaleIndex(float time);

    float GetScaleFactor(float lastTime, float nextTime, float currentTime);

    glm::vec3 InterpolatePosition(float time);
    glm::quat InterpolateRotation(float time);
    glm::vec3 InterpolateScale(float time);
};

struct BoneInfo {
    int id;
    glm::mat4 offset;
};

struct AssimpNodeData {
    std::string name;
    glm::mat4 transformation;
    std::vector<AssimpNodeData> children;
};

class Animation {
public:
	Animation() : duration(0.0f), ticksPerSecond(0.0f) {}
    Animation(aiAnimation* animation, const aiScene* scene);

    Bone* FindBone(const std::string& name);

    float GetTicksPerSecond() const { return ticksPerSecond; }
    float GetDuration() const { return duration; }
    const AssimpNodeData& GetRootNode() const { return rootNode; }
    const std::unordered_map<std::string, BoneInfo>& GetBoneIDMap() const { return boneInfoMap; }

    float duration;
    float ticksPerSecond;
    std::vector<Bone> bones;
    AssimpNodeData rootNode;
    std::unordered_map<std::string, BoneInfo> boneInfoMap;
    std::vector<glm::mat4> finalBoneMatrices;

    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);
    void ExtractBoneData(aiAnimation* animation);
    std::vector<glm::mat4> GetFinalBoneMatrices();

};

class Animator {
public:
    Animator(Animation* animation);

    void UpdateAnimation(float deltaTime);
    std::vector<glm::mat4> GetFinalBoneMatrices();

private:
    void CalculateBoneTransform(const AssimpNodeData& node, glm::mat4 parentTransform);

    std::vector<glm::mat4> finalBoneMatrices;
    Animation* currentAnimation;
    float currentTime;
    float deltaTime;
};
