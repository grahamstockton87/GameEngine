#include "AudioPlayer.h"
#include <iostream>
#include <thread>
#include <chrono>

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

AudioPlayer::AudioPlayer()
    : device(nullptr), context(nullptr), buffer(0), source(0),
    sampleRate(0), format(AL_FORMAT_STEREO16)
{
    device = alcOpenDevice(NULL);
    context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);

    alGenBuffers(1, &buffer);
    alGenSources(1, &source);
}

AudioPlayer::~AudioPlayer() {
    Cleanup();
}

bool AudioPlayer::LoadMP3(const std::string& filepath) {
    drmp3 mp3;
    if (!drmp3_init_file(&mp3, filepath.c_str(), NULL)) {
        std::cerr << "Failed to load MP3: " << filepath << std::endl;
        return false;
    }

    drmp3_uint64 frameCount = drmp3_get_pcm_frame_count(&mp3);
    pcmData.resize(frameCount * mp3.channels);
    drmp3_read_pcm_frames_s16(&mp3, frameCount, pcmData.data());
    sampleRate = mp3.sampleRate;
    format = (mp3.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

    drmp3_uninit(&mp3);

    alBufferData(buffer, format, pcmData.data(),
        static_cast<ALsizei>(pcmData.size() * sizeof(int16_t)),
        sampleRate);
    alSourcei(source, AL_BUFFER, buffer);

    return true;
}

void AudioPlayer::Play() {
    alSourcePlay(source);
}

bool AudioPlayer::IsPlaying() const {
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

void AudioPlayer::Cleanup() {
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}
