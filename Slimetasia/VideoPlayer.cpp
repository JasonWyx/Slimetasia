#include "VideoPlayer.h"

#include "Application.h"
#include "GameObject.h"

REFLECT_INIT(VideoPlayer)
REFLECT_PARENT(IComponent)
REFLECT_PROPERTY(m_VideoFileName)
REFLECT_PROPERTY(m_IsPlaying)
REFLECT_PROPERTY(m_IsLooping)
REFLECT_PROPERTY(m_FillScreen)
REFLECT_END()

VideoPlayer::VideoPlayer(GameObject* parentObject, const char* componentName)
    : IComponent(parentObject, componentName)
    , m_VideoFileName()
    , m_IsPlaying(false)
    , m_IsLooping(false)
    , m_FillScreen(false)
    , m_Transform(parentObject->GetComponent<Transform>())
    , m_PrevVideoFileName()
    , m_Video()
    , m_CurrentFrame(0)
    , m_FrameCount(0)
    , m_FrameTime(0.0f)
    , m_FrameTimer(0.0f)
    , m_FrameSize()
    , m_FrameTexture(GL_NONE)
    , m_LoadFuture()
    , m_LoadingFinished(false)
    , m_LoadingTeminate(false)
    , m_LoadedCount()
    , m_Frames()
{
    p_assert(m_Transform != nullptr);
}

VideoPlayer::~VideoPlayer()
{
    m_LoadingTeminate = true;
    // Wait for loading to terminate first
    if (m_LoadFuture.valid())
    {
        m_LoadFuture.wait();
    }
    m_Video.release();
}

void VideoPlayer::OnActive()
{
    m_OwnerObject->GetParentLayer()->GetRenderLayer().AddVideoPlayer(this);
}

void VideoPlayer::OnInactive()
{
    m_OwnerObject->GetParentLayer()->GetRenderLayer().RemoveVideoPlayer(this);
}

void VideoPlayer::OnUpdate(float dt)
{
    // Load
    if (m_PrevVideoFileName != m_VideoFileName)
    {
        m_PrevVideoFileName = m_VideoFileName;

        // Release previous video data if exists
        if (m_Video.isOpened())
        {
            m_LoadingTeminate = true;
            // Wait for loading to terminate first
            m_LoadFuture.wait();
            m_Video.release();
        }

        m_Video.open(m_VideoFileName);

        if (!m_Video.isOpened())
        {
            std::cout << "[ERROR] Unable to load video file " << m_VideoFileName << std::endl;
        }
        else
        {
            m_FrameCount = static_cast<unsigned>(m_Video.get(cv::CAP_PROP_FRAME_COUNT));
            m_FrameTime = static_cast<float>(1.0 / m_Video.get(cv::CAP_PROP_FPS));
            m_FrameSize.x = static_cast<int>(m_Video.get(cv::CAP_PROP_FRAME_WIDTH));
            m_FrameSize.y = static_cast<int>(m_Video.get(cv::CAP_PROP_FRAME_HEIGHT));
            m_Frames.resize(m_FrameCount);
            m_LoadingTeminate = false;
            m_LoadedCount = 1;

            // Load initial frame
            m_Video >> m_Frames[0];
            // Flip and change color format
            cv::cvtColor(m_Frames[0], m_Frames[0], cv::COLOR_BGR2RGB);
            cv::flip(m_Frames[0], m_Frames[0], 0);

            BuildFrameTexture();

            glTextureSubImage2D(m_FrameTexture, 0, 0, 0, m_FrameSize.x, m_FrameSize.y, GL_RGB, GL_UNSIGNED_BYTE, m_Frames[0].data);

            // Dispatch multithreaded loading of frame data
            m_LoadFuture = Application::Instance().GetThreadPool().enqueue(VideoPlayer::LoadFrames, std::ref(m_Video), std::ref(m_Frames), std::ref(m_LoadedCount), m_FrameCount,
                                                                           std::ref(m_LoadingFinished), std::cref(m_LoadingTeminate));
        }
    }

    // Update frame timer
    if (m_IsPlaying)
    {
        if (m_FrameTimer > m_FrameTime)
        {
            int framesElapsed = static_cast<int>(m_FrameTimer / m_FrameTime);
            m_FrameTimer = fmodf(m_FrameTimer, m_FrameTime);

            // End of the video
            if (m_CurrentFrame + framesElapsed >= m_FrameCount)
            {
                // Loop if enabled
                if (m_IsLooping)
                {
                    m_CurrentFrame = (m_CurrentFrame + framesElapsed) % m_FrameCount;
                }
                else
                {
                    m_IsPlaying = false;
                    m_FrameTimer = 0.0f;
                }
            }
            // Fetch next frame
            else
            {
                m_CurrentFrame += framesElapsed;
            }
        }
        m_FrameTimer += dt;
    }

    // Copy current frame into texture
    if (m_FrameCount && m_CurrentFrame < m_LoadedCount)
    {
        glTextureSubImage2D(m_FrameTexture, 0, 0, 0, m_FrameSize.x, m_FrameSize.y, GL_RGB, GL_UNSIGNED_BYTE, m_Frames[m_CurrentFrame].data);
    }
}

void VideoPlayer::Play()
{
    m_IsPlaying = true;
}

void VideoPlayer::Stop()
{
    m_IsPlaying = false;
    m_CurrentFrame = 0;
    // m_Video.set(cv::CAP_PROP_POS_FRAMES, 0.0);
}

void VideoPlayer::Pause()
{
    m_IsPlaying = false;
}

Transform* VideoPlayer::GetTransform()
{
    return m_Transform;
}

GLuint VideoPlayer::GetVideoFrameTexture() const
{
    return m_FrameTexture;
}

float VideoPlayer::GetFrameRatio() const
{
    return static_cast<float>(m_FrameSize.x) / m_FrameSize.y;
}

void VideoPlayer::BuildFrameTexture()
{
    glDeleteTextures(1, &m_FrameTexture);
    glCreateTextures(GL_TEXTURE_2D, 1, &m_FrameTexture);
    glTextureStorage2D(m_FrameTexture, 1, GL_RGB8, m_FrameSize.x, m_FrameSize.y);
    glTextureParameteri(m_FrameTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_FrameTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_FrameTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_FrameTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void VideoPlayer::LoadFrames(cv::VideoCapture& videoCapture, std::vector<cv::Mat>& frames, unsigned& loadedCount, const unsigned maxFrames, bool& loadingFinished, const bool& terminate)
{
    while (loadedCount < maxFrames)
    {
        // If quick termination is flagged
        if (terminate)
        {
            return;
        }

        videoCapture >> frames[loadedCount];
        cv::Mat& frame = frames[loadedCount];

        // Flip and change color format
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        cv::flip(frame, frame, 0);

        loadedCount++;
    }

    loadingFinished = true;
}
