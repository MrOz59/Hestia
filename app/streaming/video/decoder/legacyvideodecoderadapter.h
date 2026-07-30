#pragma once

#include "decoder.h"

#include <Limelight.h>

#include <memory>

class IVideoDecoder;

namespace Decoder {

// Owns an initialized Moonlight decoder and keeps its Limelight-specific
// submission API outside the protocol-neutral IDecoder contract.
class LegacyVideoDecoderAdapter final : public IDecoder {
public:
    explicit LegacyVideoDecoderAdapter(IVideoDecoder* decoder);
    ~LegacyVideoDecoderAdapter() override;

    const Properties& properties() const noexcept override;
    void renderFrameOnMainThread() override;
    void setHdrMode(bool enabled) override;
    bool notifyWindowChanged(
            const WindowStateChange& change) override;

    int submitDecodeUnit(PDECODE_UNIT decodeUnit);

private:
    std::unique_ptr<IVideoDecoder> m_Decoder;
    Properties m_Properties;
};

} // namespace Decoder
