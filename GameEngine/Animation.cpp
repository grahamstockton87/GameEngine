
#include "Animation.h"
#include <gtc/matrix_transform.hpp>

Bone::Bone(const std::string& name, int id, const aiNodeAnim* channel)
    : name(name), id(id)
{
    for (unsigned i = 0; i < channel->mNumPositionKeys; ++i)
        positions.push_back({ glm::vec3(channel->mPositionKeys[i].mValue.x,
                                        channel->mPositionKeys[i].mValue.y,
                                        channel->mPositionKeys[i].mValue.z),
                              (float)channel->mPositionKeys[i].mTime });

    for (unsigned i = 0; i < channel->mNumRotationKeys; ++i)
        rotations.push_back({ glm::quat(channel->mRotationKeys[i].mValue.w,
                                        channel->mRotationKeys[i].mValue.x,
                                        channel->mRotationKeys[i].mValue.y,
                                        channel->mRotationKeys[i].mValue.z),
                              (float)channel->mRotationKeys[i].mTime });

    for (unsigned i = 0; i < channel->mNumScalingKeys; ++i)
        scales.push_back({ glm::vec3(channel->mScalingKeys[i].mValue.x,
                                     channel->mScalingKeys[i].mValue.y,
                                     channel->mScalingKeys[i].mValue.z),
                           (float)channel->mScalingKeys[i].mTime });
}

void Bone::Update(float animationTime, glm::mat4& outTransform)
{
    glm::vec3 translation = InterpolatePosition(animationTime);
    glm::quat rotation = InterpolateRotation(animationTime);
    glm::vec3 scale = InterpolateScale(animationTime);

    outTransform = glm::translate(glm::mat4(1.0f), translation)
        * glm::mat4_cast(rotation)
        * glm::scale(glm::mat4(1.0f), scale);
}

// Helpers
int Bone::GetPositionIndex(float time)
{
    for (int i = 0; i < (int)positions.size() - 1; i++)
        if (time < positions[i + 1].timeStamp)
            return i;
    return (int)positions.size() - 2;
}

int Bone::GetRotationIndex(float time)
{
    for (int i = 0; i < (int)rotations.size() - 1; i++)
        if (time < rotations[i + 1].timeStamp)
            return i;
    return (int)rotations.size() - 2;
}

int Bone::GetScaleIndex(float time)
{
    for (int i = 0; i < (int)scales.size() - 1; i++)
        if (time < scales[i + 1].timeStamp)
            return i;
    return (int)scales.size() - 2;
}

float Bone::GetScaleFactor(float lastTime, float nextTime, float currentTime)
{
    float t = (currentTime - lastTime) / (nextTime - lastTime);
    return glm::clamp(t, 0.0f, 1.0f);
}

glm::vec3 Bone::InterpolatePosition(float time)
{
    if (positions.size() == 1) return positions[0].position;
    int i = GetPositionIndex(time);
    float factor = GetScaleFactor(positions[i].timeStamp, positions[i + 1].timeStamp, time);
    return glm::mix(positions[i].position, positions[i + 1].position, factor);
}

glm::quat Bone::InterpolateRotation(float time)
{
    if (rotations.size() == 1) return glm::normalize(rotations[0].orientation);
    int i = GetRotationIndex(time);
    float factor = GetScaleFactor(rotations[i].timeStamp, rotations[i + 1].timeStamp, time);
    return glm::normalize(glm::slerp(rotations[i].orientation, rotations[i + 1].orientation, factor));
}

glm::vec3 Bone::InterpolateScale(float time)
{
    if (scales.size() == 1) return scales[0].scale;
    int i = GetScaleIndex(time);
    float factor = GetScaleFactor(scales[i].timeStamp, scales[i + 1].timeStamp, time);
    return glm::mix(scales[i].scale, scales[i + 1].scale, factor);
}

Animation::Animation(aiAnimation* animation, const aiScene* scene)
{
    ticksPerSecond = animation->mTicksPerSecond != 0.0 ? animation->mTicksPerSecond : 25.0f;
    duration = animation->mDuration;

    ExtractBoneData(animation);
    ReadHierarchyData(rootNode, scene->mRootNode);
}

void Animation::ExtractBoneData(aiAnimation* animation)
{
    for (unsigned i = 0; i < animation->mNumChannels; ++i)
    {
        aiNodeAnim* channel = animation->mChannels[i];
        std::string boneName(channel->mNodeName.C_Str());

        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            BoneInfo bi;
            bi.id = (int)boneInfoMap.size();
            bi.offset = glm::mat4(1.0f); // Will be updated from mesh bones later
            boneInfoMap[boneName] = bi;
        }

        bones.emplace_back(boneName, boneInfoMap[boneName].id, channel);
    }
}

Bone* Animation::FindBone(const std::string& name)
{
    for (auto& bone : bones)
        if (bone.name == name)
            return &bone;
    return nullptr;
}

void Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
{
    dest.name = src->mName.C_Str();
    dest.transformation = glm::transpose(glm::mat4( // Replace glm::make_mat4 with glm::mat4 constructor
        src->mTransformation.a1, src->mTransformation.a2, src->mTransformation.a3, src->mTransformation.a4,
        src->mTransformation.b1, src->mTransformation.b2, src->mTransformation.b3, src->mTransformation.b4,
        src->mTransformation.c1, src->mTransformation.c2, src->mTransformation.c3, src->mTransformation.c4,
        src->mTransformation.d1, src->mTransformation.d2, src->mTransformation.d3, src->mTransformation.d4
    ));
    dest.children.resize(src->mNumChildren);

    for (unsigned i = 0; i < src->mNumChildren; ++i)
        ReadHierarchyData(dest.children[i], src->mChildren[i]);
}

std::vector<glm::mat4> Animation::GetFinalBoneMatrices()
{
    return finalBoneMatrices;
}

Animator::Animator(Animation* animation)
{
    currentAnimation = animation;
    currentTime = 0.0f;
    deltaTime = 0.0f;

    finalBoneMatrices.resize(100, glm::mat4(1.0f));
}

void Animator::UpdateAnimation(float deltaTime)
{
    if (!currentAnimation) return;

    this->deltaTime = deltaTime;
    currentTime += currentAnimation->GetTicksPerSecond() * deltaTime;
    currentTime = fmod(currentTime, currentAnimation->GetDuration());

    CalculateBoneTransform(currentAnimation->GetRootNode(), glm::mat4(1.0f));
}

void Animator::CalculateBoneTransform(const AssimpNodeData& node, glm::mat4 parentTransform)
{
    std::string nodeName = node.name;
    glm::mat4 nodeTransform = node.transformation;

    Bone* bone = currentAnimation->FindBone(nodeName);
    if (bone) {
        bone->Update(currentTime, nodeTransform);
    }

    glm::mat4 globalTransform = parentTransform * nodeTransform;

    const auto& boneInfoMap = currentAnimation->GetBoneIDMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
        int index = boneInfoMap.at(nodeName).id;
        glm::mat4 offset = boneInfoMap.at(nodeName).offset;
        if (index >= 0 && index < finalBoneMatrices.size())
            finalBoneMatrices[index] = globalTransform * offset;
    }

    for (const auto& child : node.children) {
        CalculateBoneTransform(child, globalTransform);
    }
}

std::vector<glm::mat4> Animator::GetFinalBoneMatrices()
{
    return finalBoneMatrices;
}
