#include "legacyvideodecoderadapter.h"

#include "streaming/video/decoder.h"

#include <QtGlobal>

namespace {

Decoder::ColorSpace colorSpaceFromLegacy(int colorSpace)
{
    switch (colorSpace) {
    case COLORSPACE_REC_709:
        return Decoder::ColorSpace::Rec709;
    case COLORSPACE_REC_2020:
        return Decoder::ColorSpace::Rec2020;
    case COLORSPACE_REC_601:
    default:
        return Decoder::ColorSpace::Rec601;
    }
}

Decoder::ColorRange colorRangeFromLegacy(int colorRange)
{
    return colorRange == COLOR_RANGE_FULL ?
                Decoder::ColorRange::Full :
                Decoder::ColorRange::Limited;
}

Decoder::Properties inspectDecoder(IVideoDecoder* decoder)
{
    Q_ASSERT(decoder != nullptr);

    const int legacyCapabilities = decoder->getDecoderCapabilities();
    Decoder::Properties properties;
    properties.capabilities.hardwareAccelerated =
            decoder->isHardwareAccelerated();
    properties.capabilities.alwaysFullScreen =
            decoder->isAlwaysFullScreen();
    properties.capabilities.hdr = decoder->isHdrSupported();
    properties.capabilities.directSubmit =
            legacyCapabilities & CAPABILITY_DIRECT_SUBMIT;
    properties.capabilities.pullRenderer =
            legacyCapabilities & CAPABILITY_PULL_RENDERER;
    properties.capabilities.referenceFrameInvalidationH264 =
            legacyCapabilities &
            CAPABILITY_REFERENCE_FRAME_INVALIDATION_AVC;
    properties.capabilities.referenceFrameInvalidationHevc =
            legacyCapabilities &
            CAPABILITY_REFERENCE_FRAME_INVALIDATION_HEVC;
    properties.capabilities.referenceFrameInvalidationAv1 =
            legacyCapabilities &
            CAPABILITY_REFERENCE_FRAME_INVALIDATION_AV1;
    properties.capabilities.slicesPerFrame =
            static_cast<uint8_t>(
                static_cast<uint32_t>(legacyCapabilities) >> 24);
    properties.colorSpace =
            colorSpaceFromLegacy(decoder->getDecoderColorspace());
    properties.colorRange =
            colorRangeFromLegacy(decoder->getDecoderColorRange());
    properties.maximumResolution = decoder->getDecoderMaxResolution();
    return properties;
}

} // namespace

namespace Decoder {

LegacyVideoDecoderAdapter::LegacyVideoDecoderAdapter(
        IVideoDecoder* decoder)
    : m_Decoder(decoder),
      m_Properties(inspectDecoder(m_Decoder.get()))
{
}

LegacyVideoDecoderAdapter::~LegacyVideoDecoderAdapter() = default;

const Properties&
LegacyVideoDecoderAdapter::properties() const noexcept
{
    return m_Properties;
}

void LegacyVideoDecoderAdapter::renderFrameOnMainThread()
{
    m_Decoder->renderFrameOnMainThread();
}

void LegacyVideoDecoderAdapter::setHdrMode(bool enabled)
{
    m_Decoder->setHdrMode(enabled);
}

bool LegacyVideoDecoderAdapter::notifyWindowChanged(
        const WindowStateChange& change)
{
    WINDOW_STATE_CHANGE_INFO legacyChange {};
    legacyChange.window =
            static_cast<SDL_Window*>(change.nativeWindow);
    if (change.sizeChanged) {
        legacyChange.stateChangeFlags |= WINDOW_STATE_CHANGE_SIZE;
        legacyChange.width = change.width;
        legacyChange.height = change.height;
    }
    if (change.displayChanged) {
        legacyChange.stateChangeFlags |= WINDOW_STATE_CHANGE_DISPLAY;
        legacyChange.displayIndex = change.displayIndex;
    }

    return m_Decoder->notifyWindowChanged(&legacyChange);
}

int LegacyVideoDecoderAdapter::submitDecodeUnit(
        PDECODE_UNIT decodeUnit)
{
    return m_Decoder->submitDecodeUnit(decodeUnit);
}

} // namespace Decoder
