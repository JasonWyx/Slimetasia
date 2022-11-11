#pragma once
#include <GL/glew.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "IComponent.h"
#include "ThreadPool.h"
#include "Transform.h"

class GameObject;

class VideoPlayer : public IComponent
{
public:  // Reflected variables

    std::string m_VideoFileName;
    bool m_IsPlaying;
    bool m_IsLooping;
    bool m_FillScreen;

public:

    VideoPlayer(GameObject* parentObject, const char* componentName = "VideoPlayer");
    ~VideoPlayer();

    void OnActive() override;
    void OnInactive() override;
    void OnUpdate(float dt) override;

    void Play();
    void Stop();
    void Pause();

    Transform* GetTransform();
    GLuint GetVideoFrameTexture() const;
    float GetFrameRatio() const;

    static void LoadFrames(cv::VideoCapture& videoCapture, std::vector<cv::Mat>& frames, unsigned& loadedCount, const unsigned maxFrames, bool& loadingFinished, const bool& terminate);

private:

    Transform* m_Transform;
    std::string m_PrevVideoFileName;
    cv::VideoCapture m_Video;
    unsigned m_CurrentFrame;
    unsigned m_FrameCount;
    float m_FrameTime;
    float m_FrameTimer;
    iVector2 m_FrameSize;
    GLuint m_FrameTexture;

    std::future<void> m_LoadFuture;
    bool m_LoadingFinished;
    bool m_LoadingTeminate;
    unsigned m_LoadedCount;
    std::vector<cv::Mat> m_Frames;

private:

    void BuildFrameTexture();

    REFLECT()
};
