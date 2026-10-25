#include "microphonecapture.h"

#include <Limelight.h>
#include <opus.h>

namespace {

// What the host's microphone stream expects (Apollo's protocol, as Hermes
// and Hestia-Android speak it): 20 ms of 48 kHz mono per packet.
constexpr int kSampleRate = 48000;
constexpr int kChannels = 1;
constexpr int kFrameSamples = 960;
constexpr Uint32 kFrameBytes = kFrameSamples * kChannels * sizeof(opus_int16);
// Hestia-Android's encoder settings: voice at 24 kbps with in-band FEC.
constexpr opus_int32 kBitrate = 24000;
constexpr int kExpectedLossPercent = 5;
// Keep at most this much captured audio waiting, dropping the oldest: the
// host would rather hear late speech never than hear it seconds behind.
constexpr Uint32 kMaxQueuedBytes = 4 * kFrameBytes;
// Opus never needs more than this for a 20 ms voice frame.
constexpr int kMaxPacketBytes = 1275;

}

MicrophoneCapture::MicrophoneCapture(const QString& deviceName)
    : m_DeviceName(deviceName),
      m_AudioInitialized(false),
      m_Device(0),
      m_Encoder(nullptr),
      m_Thread(nullptr)
{
    SDL_AtomicSet(&m_Stopping, 0);
}

MicrophoneCapture::~MicrophoneCapture()
{
    stop();
}

MicrophoneCapture::StartResult MicrophoneCapture::start()
{
    if (!LiIsMicrophoneStreamActive()) {
        return StartResult::NotNegotiated;
    }

    // The playback renderer holds the audio subsystem too; this reference is
    // the capture's own and is given back in stop().
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Microphone: SDL_InitSubSystem(SDL_INIT_AUDIO) failed: %s",
                     SDL_GetError());
        return StartResult::DeviceFailed;
    }
    m_AudioInitialized = true;

    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = kSampleRate;
    want.format = AUDIO_S16SYS;
    want.channels = kChannels;
    want.samples = kFrameSamples;
    // No callback: the capture thread dequeues whole frames itself. Without
    // allowed changes SDL converts whatever the device records into this.
    const QByteArray deviceName = m_DeviceName.toUtf8();
    m_Device = SDL_OpenAudioDevice(deviceName.isEmpty() ? nullptr : deviceName.constData(),
                                   1, &want, &have, 0);
    if (m_Device == 0 && !deviceName.isEmpty()) {
        // The chosen device went away since it was picked; use the default
        // rather than streaming no microphone at all.
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "Microphone: unable to open '%s' (%s); trying the default input",
                    deviceName.constData(), SDL_GetError());
        m_Device = SDL_OpenAudioDevice(nullptr, 1, &want, &have, 0);
    }
    if (m_Device == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Microphone: unable to open an input device: %s",
                     SDL_GetError());
        stop();
        return StartResult::DeviceFailed;
    }

    int error = OPUS_OK;
    m_Encoder = opus_encoder_create(kSampleRate, kChannels, OPUS_APPLICATION_VOIP, &error);
    if (m_Encoder == nullptr || error != OPUS_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Microphone: opus_encoder_create() failed: %s",
                     opus_strerror(error));
        stop();
        return StartResult::EncoderFailed;
    }
    opus_encoder_ctl(m_Encoder, OPUS_SET_BITRATE(kBitrate));
    opus_encoder_ctl(m_Encoder, OPUS_SET_VBR(1));
    opus_encoder_ctl(m_Encoder, OPUS_SET_COMPLEXITY(10));
    opus_encoder_ctl(m_Encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
    opus_encoder_ctl(m_Encoder, OPUS_SET_LSB_DEPTH(16));
    opus_encoder_ctl(m_Encoder, OPUS_SET_DTX(0));
    opus_encoder_ctl(m_Encoder, OPUS_SET_INBAND_FEC(1));
    opus_encoder_ctl(m_Encoder, OPUS_SET_PACKET_LOSS_PERC(kExpectedLossPercent));
    opus_encoder_ctl(m_Encoder, OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_20_MS));

    SDL_AtomicSet(&m_Stopping, 0);
    m_Thread = SDL_CreateThread(MicrophoneCapture::captureThread, "Microphone", this);
    if (m_Thread == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Microphone: unable to start the capture thread: %s",
                     SDL_GetError());
        stop();
        return StartResult::DeviceFailed;
    }

    SDL_PauseAudioDevice(m_Device, 0);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Microphone: streaming %s to the host (%s)",
                deviceName.isEmpty() ? "the default input" : deviceName.constData(),
                LiIsMicrophoneEncryptionEnabled() ? "encrypted" : "unencrypted");
    return StartResult::Started;
}

void MicrophoneCapture::stop()
{
    SDL_AtomicSet(&m_Stopping, 1);
    if (m_Thread != nullptr) {
        SDL_WaitThread(m_Thread, nullptr);
        m_Thread = nullptr;
    }
    if (m_Device != 0) {
        SDL_PauseAudioDevice(m_Device, 1);
        SDL_CloseAudioDevice(m_Device);
        m_Device = 0;
    }
    if (m_Encoder != nullptr) {
        opus_encoder_destroy(m_Encoder);
        m_Encoder = nullptr;
    }
    if (m_AudioInitialized) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        m_AudioInitialized = false;
    }
}

int SDLCALL MicrophoneCapture::captureThread(void* context)
{
    static_cast<MicrophoneCapture*>(context)->captureLoop();
    return 0;
}

void MicrophoneCapture::captureLoop()
{
    opus_int16 frame[kFrameSamples * kChannels];
    unsigned char packet[kMaxPacketBytes];
    bool sentFirstPacket = false;

    while (!SDL_AtomicGet(&m_Stopping)) {
        Uint32 queued = SDL_GetQueuedAudioSize(m_Device);
        if (queued < kFrameBytes) {
            SDL_Delay(5);
            continue;
        }

        // Catch up after a stall instead of carrying the delay forward.
        while (queued > kMaxQueuedBytes) {
            SDL_DequeueAudio(m_Device, frame, kFrameBytes);
            queued -= kFrameBytes;
        }

        if (SDL_DequeueAudio(m_Device, frame, kFrameBytes) != kFrameBytes) {
            continue;
        }

        const opus_int32 encoded = opus_encode(m_Encoder, frame, kFrameSamples, packet, sizeof(packet));
        if (encoded < 0) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                        "Microphone: opus_encode() failed: %s",
                        opus_strerror(encoded));
            continue;
        }

        if (LiSendMicrophoneOpusDataEx(packet, encoded, kFrameSamples) >= 0 && !sentFirstPacket) {
            sentFirstPacket = true;
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                        "Microphone: sent the first packet (%d bytes of Opus)",
                        encoded);
        }
    }
}

QStringList MicrophoneCapture::availableDevices()
{
    QStringList devices;
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "Microphone: unable to list input devices: %s",
                    SDL_GetError());
        return devices;
    }

    const int count = SDL_GetNumAudioDevices(1);
    for (int i = 0; i < count; i++) {
        const char* name = SDL_GetAudioDeviceName(i, 1);
        if (name != nullptr && !devices.contains(QString::fromUtf8(name))) {
            devices.append(QString::fromUtf8(name));
        }
    }

    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    return devices;
}
