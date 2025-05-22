#pragma once
#include <string>
#include <vector>
#include <AL/al.h>
#include <AL/alc.h>
#include "dr_mp3.h"

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();

    bool LoadMP3(const std::string& filepath);
    void Play();
    bool IsPlaying() const;

private:
    ALCdevice* device;
    ALCcontext* context;
    ALuint buffer;
    ALuint source;

    std::vector<int16_t> pcmData;
    ALsizei sampleRate;
    ALenum format;

    void Cleanup();
};
