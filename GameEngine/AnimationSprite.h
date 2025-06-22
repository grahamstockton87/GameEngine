#pragma once
#include "Sprite.h"

class AnimationSprite {
public:
	// frameCount: total frames in the sheet
	// frameDuration: seconds each frame stays on
	AnimationSprite()
		: mSprite()            // default‐constructed sprite
		, mFrameCount(0)
		, mFrameDuration(0.0f)
		, mCurrentFrame(0)
		, mElapsed(0.0f)
	{
		// nothing else to do
	} // default constructor
	AnimationSprite(Sprite& sprite, int frameCount, float frameDuration)
		: mSprite(sprite)
		, mFrameCount(frameCount)
		, mFrameDuration(frameDuration)
		, mCurrentFrame(0)
		, mElapsed(0.0f)
	{
	}
	AnimationSprite& operator=(const AnimationSprite& other) {
		if (this == &other)
			return *this;

		mSprite = other.mSprite;       // invoke Sprite’s operator=
		mFrameCount = other.mFrameCount;
		mFrameDuration = other.mFrameDuration;
		mCurrentFrame = other.mCurrentFrame;
		mElapsed = other.mElapsed;
		return *this;
	}
	// Call each tick with your delta-time
	void Update(float deltaTime) {
		mElapsed += deltaTime;
		// advance frame if long enough
		while (mElapsed >= mFrameDuration) {
			mElapsed -= mFrameDuration;
			mCurrentFrame = (mCurrentFrame + 1) % mFrameCount;
			// tell the sprite to show the new UVs
			mSprite.SetFrameUVs(mCurrentFrame, mFrameCount);
		}
	}
	// Forwarded Render
	void Render(Shader& shader, const glm::mat4& orthoProj) {
		mSprite.Render(shader, orthoProj);
	}

	void Reset() {
		mCurrentFrame = 0;
		mElapsed = 0.0f;
		mSprite.SetFrameUVs(0, mFrameCount);
	}

private:
	Sprite mSprite;
	int     mFrameCount;
	float   mFrameDuration;
	int     mCurrentFrame;
	float   mElapsed;
};
