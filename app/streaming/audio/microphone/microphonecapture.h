#pragma once

#include <QString>
#include <QStringList>

#include "SDL_compat.h"

struct OpusEncoder;

// Sends this computer's microphone to the host, the way Hestia-Android does:
// 20 ms frames of 48 kHz mono, encoded as Opus voice and handed to
// moonlight-common-c, which packetizes (and, when the host asks, encrypts)
// them on the stream the host negotiated. Hosts that did not negotiate a
// microphone stream never see it: start() then refuses.
class MicrophoneCapture
{
public:
    // An empty device name is the system default input.
    explicit MicrophoneCapture(const QString& deviceName);
    ~MicrophoneCapture();

    MicrophoneCapture(const MicrophoneCapture&) = delete;
    MicrophoneCapture& operator=(const MicrophoneCapture&) = delete;

    enum class StartResult {
        Started,
        NotNegotiated,   // The host did not set up a microphone stream.
        DeviceFailed,    // The input device could not be opened.
        EncoderFailed,
    };

    // Call once the connection is up.
    StartResult start();

    // Stops capturing; called before the connection stops.
    void stop();

    // Names of the capture devices SDL can open, for the settings page.
    static QStringList availableDevices();

private:
    static int SDLCALL captureThread(void* context);
    void captureLoop();

    QString m_DeviceName;
    bool m_AudioInitialized;
    SDL_AudioDeviceID m_Device;
    OpusEncoder* m_Encoder;
    SDL_Thread* m_Thread;
    SDL_atomic_t m_Stopping;
};
