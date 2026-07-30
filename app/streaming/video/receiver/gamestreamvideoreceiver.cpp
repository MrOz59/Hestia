#include "gamestreamvideoreceiver.h"

#include "SDL_compat.h"
#include "streaming/video/decoder/legacyvideodecoderadapter.h"

#include <QtGlobal>

namespace VideoReceiver {

GameStreamVideoReceiver* GameStreamVideoReceiver::s_ActiveReceiver = nullptr;

GameStreamVideoReceiver::GameStreamVideoReceiver(
        GameStreamVideoReceiverContext context)
    : m_Context(context),
      m_Callbacks {}
{
    Q_ASSERT(m_Context.decoder != nullptr);
    Q_ASSERT(m_Context.decoderLock != nullptr);
    Q_ASSERT(m_Context.activeVideoFormat != nullptr);
    Q_ASSERT(m_Context.activeVideoWidth != nullptr);
    Q_ASSERT(m_Context.activeVideoHeight != nullptr);
    Q_ASSERT(m_Context.activeVideoFrameRate != nullptr);

    LiInitializeVideoCallbacks(&m_Callbacks);
    m_Callbacks.setup = setupCallback;
    m_Callbacks.submitDecodeUnit = submitDecodeUnitCallback;
}

GameStreamVideoReceiver::~GameStreamVideoReceiver()
{
    Q_ASSERT(s_ActiveReceiver != this);
    if (s_ActiveReceiver == this) {
        s_ActiveReceiver = nullptr;
    }
}

Mode GameStreamVideoReceiver::mode() const noexcept
{
    return Mode::GameStream;
}

Capabilities GameStreamVideoReceiver::capabilities() const noexcept
{
    return m_Capabilities;
}

void GameStreamVideoReceiver::configure(
        Capabilities capabilities) noexcept
{
    m_Capabilities = capabilities;

    int legacyCapabilities = 0;
    if (capabilities.directSubmit) {
        legacyCapabilities |= CAPABILITY_DIRECT_SUBMIT;
    }
    if (capabilities.pullRenderer) {
        legacyCapabilities |= CAPABILITY_PULL_RENDERER;
    }
    if (capabilities.referenceFrameInvalidationH264) {
        legacyCapabilities |= CAPABILITY_REFERENCE_FRAME_INVALIDATION_AVC;
    }
    if (capabilities.referenceFrameInvalidationHevc) {
        legacyCapabilities |= CAPABILITY_REFERENCE_FRAME_INVALIDATION_HEVC;
    }
    if (capabilities.referenceFrameInvalidationAv1) {
        legacyCapabilities |= CAPABILITY_REFERENCE_FRAME_INVALIDATION_AV1;
    }
    if (capabilities.slicesPerFrame != 0) {
        legacyCapabilities |=
                CAPABILITY_SLICES_PER_FRAME(capabilities.slicesPerFrame);
    }

    m_Callbacks.capabilities = legacyCapabilities;
    m_Callbacks.submitDecodeUnit =
            capabilities.pullRenderer ?
                nullptr :
                submitDecodeUnitCallback;
}

bool GameStreamVideoReceiver::activate() noexcept
{
    if (s_ActiveReceiver != nullptr && s_ActiveReceiver != this) {
        return false;
    }

    s_ActiveReceiver = this;
    return true;
}

void GameStreamVideoReceiver::deactivate() noexcept
{
    if (s_ActiveReceiver == this) {
        s_ActiveReceiver = nullptr;
    }
}

PDECODER_RENDERER_CALLBACKS
GameStreamVideoReceiver::callbacks() noexcept
{
    return &m_Callbacks;
}

int GameStreamVideoReceiver::setupCallback(
        int videoFormat,
        int width,
        int height,
        int frameRate,
        void*,
        int)
{
    Q_ASSERT(s_ActiveReceiver != nullptr);
    return s_ActiveReceiver != nullptr ?
                s_ActiveReceiver->setup(
                    videoFormat,
                    width,
                    height,
                    frameRate) :
                -1;
}

int GameStreamVideoReceiver::submitDecodeUnitCallback(
        PDECODE_UNIT decodeUnit)
{
    Q_ASSERT(s_ActiveReceiver != nullptr);
    return s_ActiveReceiver != nullptr ?
                s_ActiveReceiver->submitDecodeUnit(decodeUnit) :
                DR_OK;
}

int GameStreamVideoReceiver::setup(
        int videoFormat,
        int width,
        int height,
        int frameRate)
{
    *m_Context.activeVideoFormat = videoFormat;
    *m_Context.activeVideoWidth = width;
    *m_Context.activeVideoHeight = height;
    *m_Context.activeVideoFrameRate = frameRate;

    // Decoder creation remains deferred until the SDL streaming loop starts.
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Video stream is %dx%dx%d (format 0x%x)",
                width,
                height,
                frameRate,
                videoFormat);
    return 0;
}

int GameStreamVideoReceiver::submitDecodeUnit(
        PDECODE_UNIT decodeUnit)
{
    // The decoder is destroyed and recreated on the main thread. If the lock
    // is busy, accepting and dropping this frame is safe because recreation
    // requests a fresh IDR frame.
    if (SDL_TryLockMutex(m_Context.decoderLock) != 0) {
        return DR_OK;
    }

    Decoder::IDecoder* decoder = *m_Context.decoder;
    auto* legacyDecoder =
            dynamic_cast<Decoder::LegacyVideoDecoderAdapter*>(decoder);
    const int result =
            legacyDecoder != nullptr ?
                legacyDecoder->submitDecodeUnit(decodeUnit) :
                DR_OK;
    SDL_UnlockMutex(m_Context.decoderLock);
    return result;
}

} // namespace VideoReceiver
