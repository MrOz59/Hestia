#include "pyrowavedecoder.h"

#include "streaming/session.h"

#include <Limelight.h>
#include <vulkan/vulkan.h>
#include <pyrowave.h>

#include <utility>

namespace {

bool checkPyroWave(pyrowave_result result, const char* operation)
{
    if (result == PYROWAVE_SUCCESS) {
        return true;
    }

    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "PyroWave %s failed: %d",
                 operation,
                 static_cast<int>(result));
    return false;
}

} // namespace

PyroWaveVideoDecoder::PyroWaveVideoDecoder()
    : m_Device(nullptr),
      m_Decoder(nullptr),
      m_Renderer(nullptr),
      m_Texture(nullptr),
      m_FrameLock(SDL_CreateMutex()),
      m_Width(0),
      m_Height(0),
      m_TestOnly(false),
      m_FrameEventPending(false),
      m_HasPendingFrame(false)
{
}

PyroWaveVideoDecoder::~PyroWaveVideoDecoder()
{
    if (m_Texture != nullptr) {
        SDL_DestroyTexture(m_Texture);
    }
    if (m_Renderer != nullptr) {
        SDL_DestroyRenderer(m_Renderer);
    }
    if (m_Decoder != nullptr) {
        pyrowave_decoder_destroy(m_Decoder);
    }
    if (m_Device != nullptr) {
        pyrowave_device_destroy(m_Device);
    }
    if (m_FrameLock != nullptr) {
        SDL_DestroyMutex(m_FrameLock);
    }
}

bool PyroWaveVideoDecoder::initialize(PDECODER_PARAMETERS params)
{
    if ((params->videoFormat & VIDEO_FORMAT_MASK_PYROWAVE) == 0 ||
            params->width <= 0 || params->height <= 0 ||
            (params->width & 1) != 0 || (params->height & 1) != 0 ||
            m_FrameLock == nullptr) {
        return false;
    }

    uint32_t major = 0;
    uint32_t minor = 0;
    uint32_t patch = 0;
    pyrowave_get_api_version(&major, &minor, &patch);
    // Every 0.x release shares the soname, yet 0.x promises no ABI
    // stability, so until 1.0 only the minor version Hestia was built
    // against is safe to call into.
    const bool compatible = major == PYROWAVE_API_VERSION_MAJOR &&
            (major == 0 ? minor == PYROWAVE_API_VERSION_MINOR
                        : minor >= PYROWAVE_API_VERSION_MINOR);
    if (!compatible) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Incompatible PyroWave API %u.%u.%u (need %u.%u.x)",
                     major,
                     minor,
                     patch,
                     PYROWAVE_API_VERSION_MAJOR,
                     PYROWAVE_API_VERSION_MINOR);
        return false;
    }

    if (!checkPyroWave(pyrowave_create_default_device(&m_Device),
                       "device creation")) {
        return false;
    }

    pyrowave_decoder_create_info decoderInfo {};
    decoderInfo.device = m_Device;
    decoderInfo.width = params->width;
    decoderInfo.height = params->height;
    decoderInfo.chroma = PYROWAVE_CHROMA_SUBSAMPLING_420;
    decoderInfo.fragment_path =
            pyrowave_decoder_device_prefers_fragment_path(m_Device);
    if (!checkPyroWave(pyrowave_decoder_create(&decoderInfo, &m_Decoder),
                       "decoder creation")) {
        return false;
    }

    m_Width = params->width;
    m_Height = params->height;
    m_TestOnly = params->testOnly;
    const size_t lumaSize = static_cast<size_t>(m_Width) * m_Height;
    m_DecodeBuffer.resize(lumaSize + lumaSize / 2);

    if (m_TestOnly) {
        return true;
    }

    m_PendingFrame.resize(m_DecodeBuffer.size());
    m_RenderFrame.resize(m_DecodeBuffer.size());

    Uint32 rendererFlags = SDL_RENDERER_ACCELERATED;
    if (params->enableVsync) {
        rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
    }
    m_Renderer = SDL_CreateRenderer(params->window, -1, rendererFlags);
    if (m_Renderer == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL_CreateRenderer() for PyroWave failed: %s",
                     SDL_GetError());
        return false;
    }

    // The host encodes with the colorspace this decoder reports (Rec. 709,
    // limited range). SDL picks the matrix for a YUV texture when it is
    // created, and its default is Rec. 601, which shifts every colour.
    SDL_SetYUVConversionMode(SDL_YUV_CONVERSION_BT709);
    m_Texture = SDL_CreateTexture(m_Renderer,
                                  SDL_PIXELFORMAT_IYUV,
                                  SDL_TEXTUREACCESS_STREAMING,
                                  m_Width,
                                  m_Height);
    if (m_Texture == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL_CreateTexture() for PyroWave failed: %s",
                     SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(m_Renderer);
    SDL_RenderPresent(m_Renderer);
    return true;
}

bool PyroWaveVideoDecoder::isHardwareAccelerated()
{
    return true;
}

bool PyroWaveVideoDecoder::isAlwaysFullScreen()
{
    return false;
}

bool PyroWaveVideoDecoder::isHdrSupported()
{
    return false;
}

int PyroWaveVideoDecoder::getDecoderCapabilities()
{
    return 0;
}

int PyroWaveVideoDecoder::getDecoderColorspace()
{
    return COLORSPACE_REC_709;
}

int PyroWaveVideoDecoder::getDecoderColorRange()
{
    return COLOR_RANGE_LIMITED;
}

QSize PyroWaveVideoDecoder::getDecoderMaxResolution()
{
    return QSize(8192, 8192);
}

int PyroWaveVideoDecoder::submitDecodeUnit(PDECODE_UNIT decodeUnit)
{
    if (decodeUnit == nullptr || decodeUnit->fullLength <= 0 ||
            decodeUnit->bufferList == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "PyroWave received an invalid decode unit");
        return DR_NEED_IDR;
    }

    pyrowave_decoder_clear(m_Decoder);

    // A frame normally arrives as one buffer and is decoded where it lies.
    const uint8_t* encoded = reinterpret_cast<const uint8_t*>(decodeUnit->bufferList->data);
    size_t encodedSize = static_cast<size_t>(decodeUnit->bufferList->length);
    if (decodeUnit->bufferList->next != nullptr) {
        m_EncodedFrame.clear();
        for (PLENTRY entry = decodeUnit->bufferList;
             entry != nullptr;
             entry = entry->next) {
            const auto* begin = reinterpret_cast<const uint8_t*>(entry->data);
            m_EncodedFrame.insert(m_EncodedFrame.end(),
                                  begin,
                                  begin + entry->length);
        }
        encoded = m_EncodedFrame.data();
        encodedSize = m_EncodedFrame.size();
    }
    if (encodedSize == 0 ||
            !checkPyroWave(
                pyrowave_decoder_push_packet(m_Decoder,
                                             encoded,
                                             encodedSize),
                "frame submission")) {
        return DR_NEED_IDR;
    }

    if (!pyrowave_decoder_decode_is_ready(m_Decoder, false)) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "Incomplete PyroWave frame %d",
                    decodeUnit->frameNumber);
        return DR_NEED_IDR;
    }

    const size_t lumaSize = static_cast<size_t>(m_Width) * m_Height;
    const size_t chromaSize = lumaSize / 4;
    pyrowave_cpu_buffer output {};
    output.data[0] = m_DecodeBuffer.data();
    output.data[1] = m_DecodeBuffer.data() + lumaSize;
    output.data[2] = m_DecodeBuffer.data() + lumaSize + chromaSize;
    output.row_stride_in_bytes[0] = static_cast<size_t>(m_Width);
    output.row_stride_in_bytes[1] = static_cast<size_t>(m_Width / 2);
    output.row_stride_in_bytes[2] = static_cast<size_t>(m_Width / 2);
    output.plane_size_in_bytes[0] = lumaSize;
    output.plane_size_in_bytes[1] = chromaSize;
    output.plane_size_in_bytes[2] = chromaSize;
    output.width = m_Width;
    output.height = m_Height;
    output.format = PYROWAVE_CPU_BUFFER_FORMAT_YUV420P;

    if (!checkPyroWave(
                pyrowave_decoder_decode_cpu_buffer_synchronous(m_Decoder,
                                                               &output),
                "frame decode")) {
        return DR_NEED_IDR;
    }

    if (m_TestOnly) {
        return DR_OK;
    }

    bool pushEvent = false;
    SDL_LockMutex(m_FrameLock);
    // A frame the main thread has not taken yet is simply replaced.
    std::swap(m_DecodeBuffer, m_PendingFrame);
    m_HasPendingFrame = true;
    if (!m_FrameEventPending) {
        m_FrameEventPending = true;
        pushEvent = true;
    }
    SDL_UnlockMutex(m_FrameLock);

    if (pushEvent) {
        SDL_Event event {};
        event.type = SDL_USEREVENT;
        event.user.code = SDL_CODE_FRAME_READY;
        if (SDL_PushEvent(&event) < 0) {
            SDL_LockMutex(m_FrameLock);
            m_FrameEventPending = false;
            SDL_UnlockMutex(m_FrameLock);
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Unable to queue PyroWave frame: %s",
                         SDL_GetError());
        }
    }
    return DR_OK;
}

void PyroWaveVideoDecoder::renderFrameOnMainThread()
{
    bool haveFrame;
    SDL_LockMutex(m_FrameLock);
    m_FrameEventPending = false;
    haveFrame = m_HasPendingFrame;
    if (haveFrame) {
        std::swap(m_RenderFrame, m_PendingFrame);
        m_HasPendingFrame = false;
    }
    SDL_UnlockMutex(m_FrameLock);

    if (!haveFrame || m_Texture == nullptr || m_Renderer == nullptr) {
        return;
    }

    const std::vector<uint8_t>& frame = m_RenderFrame;

    const size_t lumaSize = static_cast<size_t>(m_Width) * m_Height;
    const size_t chromaSize = lumaSize / 4;
    if (SDL_UpdateYUVTexture(m_Texture,
                             nullptr,
                             frame.data(),
                             m_Width,
                             frame.data() + lumaSize,
                             m_Width / 2,
                             frame.data() + lumaSize + chromaSize,
                             m_Width / 2) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL_UpdateYUVTexture() for PyroWave failed: %s",
                     SDL_GetError());
        return;
    }

    SDL_RenderClear(m_Renderer);
    SDL_RenderCopy(m_Renderer, m_Texture, nullptr, nullptr);
    SDL_RenderPresent(m_Renderer);
}

void PyroWaveVideoDecoder::setHdrMode(bool)
{
}

bool PyroWaveVideoDecoder::notifyWindowChanged(PWINDOW_STATE_CHANGE_INFO)
{
    return false;
}
