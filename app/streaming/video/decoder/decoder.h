#pragma once

#include <QSize>

#include <cstdint>

namespace Decoder {

enum class ColorSpace : uint8_t {
    Rec601 = 0,
    Rec709 = 1,
    Rec2020 = 2,
};

enum class ColorRange : uint8_t {
    Limited = 0,
    Full = 1,
};

struct Capabilities {
    bool hardwareAccelerated = false;
    bool alwaysFullScreen = false;
    bool hdr = false;
    bool directSubmit = false;
    bool pullRenderer = false;
    bool referenceFrameInvalidationH264 = false;
    bool referenceFrameInvalidationHevc = false;
    bool referenceFrameInvalidationAv1 = false;
    uint8_t slicesPerFrame = 0;
};

struct Properties {
    Capabilities capabilities;
    ColorSpace colorSpace = ColorSpace::Rec601;
    ColorRange colorRange = ColorRange::Limited;
    QSize maximumResolution;
};

struct WindowStateChange {
    // Opaque platform window owned by the caller.
    void* nativeWindow = nullptr;
    bool sizeChanged = false;
    bool displayChanged = false;
    int width = 0;
    int height = 0;
    int displayIndex = 0;
};

class IDecoder {
public:
    virtual ~IDecoder() = default;

    virtual const Properties& properties() const noexcept = 0;
    virtual void renderFrameOnMainThread() = 0;
    virtual void setHdrMode(bool enabled) = 0;
    virtual bool notifyWindowChanged(
            const WindowStateChange& change) = 0;
};

} // namespace Decoder
