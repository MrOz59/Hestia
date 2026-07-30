#pragma once

#include "videoreceiver.h"

#include <Limelight.h>

struct SDL_mutex;

namespace Decoder {
class IDecoder;
}

namespace VideoReceiver {

// Non-owning decoder state remains valid for the complete Session lifetime.
struct GameStreamVideoReceiverContext {
    Decoder::IDecoder** decoder = nullptr;
    SDL_mutex* decoderLock = nullptr;
    int* activeVideoFormat = nullptr;
    int* activeVideoWidth = nullptr;
    int* activeVideoHeight = nullptr;
    int* activeVideoFrameRate = nullptr;
};

class GameStreamVideoReceiver final : public IVideoReceiver {
public:
    explicit GameStreamVideoReceiver(
            GameStreamVideoReceiverContext context);
    ~GameStreamVideoReceiver() override;

    Mode mode() const noexcept override;
    Capabilities capabilities() const noexcept override;
    void configure(Capabilities capabilities) noexcept override;

    bool activate() noexcept;
    void deactivate() noexcept;
    PDECODER_RENDERER_CALLBACKS callbacks() noexcept;

private:
    static int setupCallback(
            int videoFormat,
            int width,
            int height,
            int frameRate,
            void* context,
            int flags);
    static int submitDecodeUnitCallback(PDECODE_UNIT decodeUnit);

    int setup(int videoFormat, int width, int height, int frameRate);
    int submitDecodeUnit(PDECODE_UNIT decodeUnit);

    GameStreamVideoReceiverContext m_Context;
    Capabilities m_Capabilities;
    DECODER_RENDERER_CALLBACKS m_Callbacks;

    static GameStreamVideoReceiver* s_ActiveReceiver;
};

} // namespace VideoReceiver
