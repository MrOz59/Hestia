#pragma once

#include <QString>

#include <cstdint>
#include <utility>

namespace RenderScheduler {

struct Configuration {
    // Opaque platform window owned by the caller.
    void* nativeWindow = nullptr;
    uint32_t maximumFrameRate = 0;
    bool framePacing = false;
};

struct NetworkConditions {
    uint32_t roundTripTimeMs = 0;
    uint32_t roundTripTimeVarianceMs = 0;
};

struct Capabilities {
    uint32_t maximumOutstandingFrames = 0;
};

struct FrameTiming {
    int64_t frameNumber = -1;
    // Client monotonic time when decode completed.
    uint64_t readyTimeUs = 0;
    // Source-relative presentation timeline. The epoch need not match the
    // client's monotonic clock.
    uint64_t sourcePresentationTimeUs = 0;
    // Presentation deadline mapped onto the client's monotonic clock.
    // Zero when the active protocol does not provide one.
    uint64_t presentationDeadlineUs = 0;
    uint32_t rtpTimestamp = 0;
};

using NativeFrameReleaser = void (*)(void* nativeHandle) noexcept;

// Move-only frame ownership transferred from a decoder to the scheduler.
// Native handles allow zero-copy platform frames without coupling this
// contract to FFmpeg, VA-API, VideoToolbox, MediaCodec, or D3D.
class DecodedFrame {
public:
    DecodedFrame() noexcept = default;

    DecodedFrame(void* nativeHandle,
                 NativeFrameReleaser releaser,
                 FrameTiming timing = {}) noexcept
        : m_NativeHandle(nativeHandle),
          m_Releaser(releaser),
          m_Timing(timing)
    {
    }

    ~DecodedFrame()
    {
        reset();
    }

    DecodedFrame(const DecodedFrame&) = delete;
    DecodedFrame& operator=(const DecodedFrame&) = delete;

    DecodedFrame(DecodedFrame&& other) noexcept
        : m_NativeHandle(
              std::exchange(other.m_NativeHandle, nullptr)),
          m_Releaser(std::exchange(other.m_Releaser, nullptr)),
          m_Timing(other.m_Timing)
    {
    }

    DecodedFrame& operator=(DecodedFrame&& other) noexcept
    {
        if (this != &other) {
            reset();
            m_NativeHandle =
                    std::exchange(other.m_NativeHandle, nullptr);
            m_Releaser =
                    std::exchange(other.m_Releaser, nullptr);
            m_Timing = other.m_Timing;
        }
        return *this;
    }

    explicit operator bool() const noexcept
    {
        return m_NativeHandle != nullptr && m_Releaser != nullptr;
    }

    const FrameTiming& timing() const noexcept
    {
        return m_Timing;
    }

    void* takeNativeHandle() noexcept
    {
        m_Releaser = nullptr;
        return std::exchange(m_NativeHandle, nullptr);
    }

    void reset() noexcept
    {
        if (m_NativeHandle != nullptr && m_Releaser != nullptr) {
            m_Releaser(m_NativeHandle);
        }
        m_NativeHandle = nullptr;
        m_Releaser = nullptr;
    }

private:
    void* m_NativeHandle = nullptr;
    NativeFrameReleaser m_Releaser = nullptr;
    FrameTiming m_Timing;
};

enum class PresentationPath : uint8_t {
    SynchronizedSource,
    RendererVsync,
    RendererDriven,
};

struct PresentationStatus {
    PresentationPath path = PresentationPath::RendererDriven;
    QString pathName;
    uint32_t displayRefreshRateMillihertz = 0;
};

class IRenderScheduler {
public:
    virtual ~IRenderScheduler() = default;

    virtual bool initialize(
            const Configuration& configuration) = 0;
    virtual Capabilities capabilities() const noexcept = 0;
    virtual void submitFrame(DecodedFrame frame) = 0;
    virtual void renderOnMainThread() = 0;
    virtual void updateNetworkConditions(
            NetworkConditions conditions) noexcept = 0;
    virtual PresentationStatus status() const = 0;
};

} // namespace RenderScheduler
