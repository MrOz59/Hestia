#pragma once

#include "streaming/video/decoder.h"

#include <SDL.h>

#include <cstdint>
#include <vector>

struct pyrowave_decoder_opaque;
struct pyrowave_device_opaque;

class PyroWaveVideoDecoder final : public IVideoDecoder
{
public:
    PyroWaveVideoDecoder();
    ~PyroWaveVideoDecoder() override;

    bool initialize(PDECODER_PARAMETERS params) override;
    bool isHardwareAccelerated() override;
    bool isAlwaysFullScreen() override;
    bool isHdrSupported() override;
    int getDecoderCapabilities() override;
    int getDecoderColorspace() override;
    int getDecoderColorRange() override;
    QSize getDecoderMaxResolution() override;
    int submitDecodeUnit(PDECODE_UNIT decodeUnit) override;
    void renderFrameOnMainThread() override;
    void setHdrMode(bool enabled) override;
    bool notifyWindowChanged(PWINDOW_STATE_CHANGE_INFO info) override;

private:
    pyrowave_device_opaque* m_Device;
    pyrowave_decoder_opaque* m_Decoder;
    SDL_Renderer* m_Renderer;
    SDL_Texture* m_Texture;
    SDL_mutex* m_FrameLock;
    int m_Width;
    int m_Height;
    bool m_TestOnly;
    bool m_FrameEventPending;
    bool m_HasPendingFrame;
    // Three frames that trade places instead of being copied: the decoder
    // thread writes one, the main thread uploads another, and the newest
    // finished frame waits in between under m_FrameLock.
    std::vector<uint8_t> m_DecodeBuffer;
    std::vector<uint8_t> m_PendingFrame;
    std::vector<uint8_t> m_RenderFrame;
    // Only used when a frame arrives split across several buffers.
    std::vector<uint8_t> m_EncodedFrame;
};
