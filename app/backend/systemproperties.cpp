#include "systemproperties.h"
#include "utils.h"

#include <QGuiApplication>
#include <QLibraryInfo>
#include <QByteArray>
#include <QFile>

#include "streaming/session.h"
#include "streaming/streamutils.h"

#ifdef Q_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

class SystemPropertyQueryThread : public QThread
{
public:
    SystemPropertyQueryThread(SystemProperties* properties)
        : QThread(properties), m_Properties(properties)
    {
        setObjectName("System Properties Async Query Thread");
    }

private:
    void run() override
    {
        bool hasHardwareAcceleration;
        bool rendererAlwaysFullScreen;
        bool supportsHdr;
        QSize maximumResolution;

        Session::getDecoderInfo(m_Properties->testWindow, hasHardwareAcceleration, rendererAlwaysFullScreen, supportsHdr, maximumResolution);

        // Propagate the decoder properties to the SystemProperties singleton and emit any change signals on the main thread
        QMetaObject::invokeMethod(m_Properties, "updateDecoderProperties",
                                  Qt::QueuedConnection,
                                  Q_ARG(bool, hasHardwareAcceleration),
                                  Q_ARG(bool, rendererAlwaysFullScreen),
                                  Q_ARG(QSize, maximumResolution),
                                  Q_ARG(bool, supportsHdr));
    }

private:
    SystemProperties* m_Properties;
};

// Best-effort detection of a handheld gaming device (Steam Deck and similar).
// Used to surface handheld-oriented options (e.g. the Battery Saver preset)
// only where they make sense. Conservative: unknown platforms return false.
static bool detectHandheld()
{
    // Steam exports this on Deck and on third-party handhelds running its OS image.
    if (qEnvironmentVariableIntValue("SteamDeck") == 1) {
        return true;
    }

#ifdef Q_OS_LINUX
    auto readDmi = [](const char* path) -> QByteArray {
        QFile f(QString::fromLatin1(path));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return {};
        }
        return f.readAll().trimmed();
    };

    // Steam Deck DMI product names: LCD model is "Jupiter", OLED is "Galileo".
    const QByteArray product = readDmi("/sys/class/dmi/id/product_name").toLower();
    if (product == "jupiter" || product == "galileo") {
        return true;
    }

    // Generic chassis-type signal from SMBIOS. 30=Tablet, 31=Convertible,
    // 32=Detachable are the form factors used by handheld PCs.
    bool ok = false;
    const int chassis = readDmi("/sys/class/dmi/id/chassis_type").toInt(&ok);
    if (ok && (chassis == 30 || chassis == 31 || chassis == 32)) {
        return true;
    }
#endif

    return false;
}

SystemProperties::SystemProperties()
{
    versionString = QString(VERSION_STR);
    hasDesktopEnvironment = WMUtils::isRunningDesktopEnvironment();
    isRunningWayland = WMUtils::isRunningWayland();
    isRunningXWayland = isRunningWayland && QGuiApplication::platformName() == "xcb";
    usesMaterial3Theme = QLibraryInfo::version() >= QVersionNumber(6, 5, 0);
    QString nativeArch = QSysInfo::currentCpuArchitecture();

#ifdef Q_OS_WIN32
    {
        USHORT processArch, machineArch;

        // Use IsWow64Process2() because it doesn't lie on ARM64
        if (IsWow64Process2(GetCurrentProcess(), &processArch, &machineArch)) {
            switch (machineArch) {
            case IMAGE_FILE_MACHINE_I386:
                nativeArch = "i386";
                break;
            case IMAGE_FILE_MACHINE_AMD64:
                nativeArch = "x86_64";
                break;
            case IMAGE_FILE_MACHINE_ARM64:
                nativeArch = "arm64";
                break;
            }
        }

        isWow64 = nativeArch != QSysInfo::buildCpuArchitecture();
    }
#else
    isWow64 = false;
#endif

    if (nativeArch == "i386") {
        friendlyNativeArchName = "x86";
    }
    else if (nativeArch == "x86_64") {
        friendlyNativeArchName = "x64";
    }
    else {
        friendlyNativeArchName = nativeArch.toUpper();
    }

    // Assume we can probably launch a browser if we're in a GUI environment
    hasBrowser = hasDesktopEnvironment;

    isHandheld = detectHandheld();

#ifdef HAVE_DISCORD
    hasDiscordIntegration = true;
#else
    hasDiscordIntegration = false;
#endif

    // These will be queried asynchronously to avoid blocking the UI
    hasHardwareAcceleration = true;
    rendererAlwaysFullScreen = false;
    supportsHdr = true;
    maximumResolution = QSize(0, 0);
}

SystemProperties::~SystemProperties()
{
    waitForAsyncLoad();
}

void SystemProperties::updateDecoderProperties(bool hasHardwareAcceleration, bool rendererAlwaysFullScreen, QSize maximumResolution, bool supportsHdr)
{
    SDL_assert(testWindow);

    if (hasHardwareAcceleration != this->hasHardwareAcceleration) {
        this->hasHardwareAcceleration = hasHardwareAcceleration;
        emit hasHardwareAccelerationChanged();
    }

    if (rendererAlwaysFullScreen != this->rendererAlwaysFullScreen) {
        this->rendererAlwaysFullScreen = rendererAlwaysFullScreen;
        emit rendererAlwaysFullScreenChanged();
    }

    if (maximumResolution != this->maximumResolution) {
        this->maximumResolution = maximumResolution;
        emit maximumResolutionChanged();
    }

    if (supportsHdr != this->supportsHdr) {
        this->supportsHdr = supportsHdr;
        emit supportsHdrChanged();
    }

    SDL_DestroyWindow(testWindow);
    testWindow = nullptr;
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

QRect SystemProperties::getNativeResolution(int displayIndex)
{
    // Returns default constructed QRect if out of bounds
    Q_ASSERT(!monitorNativeResolutions.isEmpty());
    return monitorNativeResolutions.value(displayIndex);
}

QRect SystemProperties::getSafeAreaResolution(int displayIndex)
{
    // Returns default constructed QRect if out of bounds
    Q_ASSERT(!monitorSafeAreaResolutions.isEmpty());
    return monitorSafeAreaResolutions.value(displayIndex);
}

int SystemProperties::getRefreshRate(int displayIndex)
{
    // Returns 0 if out of bounds
    Q_ASSERT(!monitorRefreshRates.isEmpty());
    return monitorRefreshRates.value(displayIndex);
}

QString SystemProperties::probeCodecAvailability(const QString& codec, bool hdr, bool yuv444,
                                                 int width, int height, int fps)
{
    const QString cacheKey = QString("%1|%2|%3|%4x%5@%6")
            .arg(codec.toLower()).arg(hdr).arg(yuv444).arg(width).arg(height).arg(fps);

    auto cached = codecAvailabilityCache.constFind(cacheKey);
    if (cached != codecAvailabilityCache.constEnd()) {
        return cached.value();
    }

    if (testWindow == nullptr) {
        // No test window yet (async load not completed). Don't cache this so a
        // later call once the window exists can probe for real.
        return QStringLiteral("unknown");
    }

    // Map the codec name + flags to the corresponding Limelight video format,
    // mirroring the selection logic used when building the stream's format list.
    int videoFormat;
    const QString lowerCodec = codec.toLower();
    if (lowerCodec == "h264") {
        // H.264 has no 10-bit profile; HDR is ignored here.
        videoFormat = yuv444 ? VIDEO_FORMAT_H264_HIGH8_444 : VIDEO_FORMAT_H264;
    }
    else if (lowerCodec == "hevc") {
        if (yuv444) {
            videoFormat = hdr ? VIDEO_FORMAT_H265_REXT10_444 : VIDEO_FORMAT_H265_REXT8_444;
        }
        else {
            videoFormat = hdr ? VIDEO_FORMAT_H265_MAIN10 : VIDEO_FORMAT_H265;
        }
    }
    else if (lowerCodec == "av1") {
        if (yuv444) {
            videoFormat = hdr ? VIDEO_FORMAT_AV1_HIGH10_444 : VIDEO_FORMAT_AV1_HIGH8_444;
        }
        else {
            videoFormat = hdr ? VIDEO_FORMAT_AV1_MAIN10 : VIDEO_FORMAT_AV1_MAIN8;
        }
    }
    else {
        return QStringLiteral("none");
    }

    // Probe using the user's decoder selection so a "force software" preference
    // is reflected in the result the GUI shows.
    StreamingPreferences* prefs = StreamingPreferences::get();
    Session::DecoderAvailability availability =
            Session::getDecoderAvailability(testWindow, prefs->videoDecoderSelection,
                                            videoFormat, width, height, fps);

    QString result;
    switch (availability) {
    case Session::DecoderAvailability::Hardware: result = QStringLiteral("hardware"); break;
    case Session::DecoderAvailability::Software: result = QStringLiteral("software"); break;
    default:                                     result = QStringLiteral("none");     break;
    }

    codecAvailabilityCache.insert(cacheKey, result);
    return result;
}

void SystemProperties::startAsyncLoad()
{
    if (systemPropertyQueryThread) {
        // Already started/completed
        return;
    }

    // This isn't actually asynchronous (due to the need to synchronize with
    // SdlGamepadKeyNavigation), but we don't query it in the constructor
    // because it's expensive.
    unmappedGamepads = SdlInputHandler::getUnmappedGamepads();
    if (!unmappedGamepads.isEmpty()) {
        emit unmappedGamepadsChanged();
    }

    // We initialize the video subsystem and test window on the main thread
    // because some platforms (macOS) do not support window creation on
    // non-main threads.
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL_InitSubSystem(SDL_INIT_VIDEO) failed: %s",
                     SDL_GetError());
        return;
    }

    testWindow = StreamUtils::createTestWindow();
    if (!testWindow) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to create window for hardware decode test: %s",
                     SDL_GetError());
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        return;
    }

    // Update display related attributes (max FPS, native resolution, etc).
    //
    // NB: SDL3 will forcefully refresh displays when a window is created,
    // so we place this after the window creation to ensure we don't pay
    // the penalty for mode enumeration twice.
    refreshDisplays();

    systemPropertyQueryThread = new SystemPropertyQueryThread(this);
    systemPropertyQueryThread->start();
}

void SystemProperties::waitForAsyncLoad()
{
    if (systemPropertyQueryThread) {
        systemPropertyQueryThread->wait();
    }
}

void SystemProperties::refreshDisplays()
{
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL_InitSubSystem(SDL_INIT_VIDEO) failed: %s",
                     SDL_GetError());
        return;
    }

    monitorNativeResolutions.clear();

    SDL_DisplayMode bestMode;
    for (int displayIndex = 0; displayIndex < SDL_GetNumVideoDisplays(); displayIndex++) {
        SDL_DisplayMode desktopMode;
        SDL_Rect safeArea;

        if (StreamUtils::getNativeDesktopMode(displayIndex, &desktopMode, &safeArea)) {
            if (desktopMode.w <= 8192 && desktopMode.h <= 8192) {
                monitorNativeResolutions.insert(displayIndex, QRect(0, 0, desktopMode.w, desktopMode.h));
                monitorSafeAreaResolutions.insert(displayIndex, QRect(0, 0, safeArea.w, safeArea.h));
            }
            else {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                            "Skipping resolution over 8K: %dx%d",
                            desktopMode.w, desktopMode.h);
            }

            // Start at desktop mode and work our way up
            bestMode = desktopMode;
            int numDisplayModes = SDL_GetNumDisplayModes(displayIndex);
            for (int i = 0; i < numDisplayModes; i++) {
                SDL_DisplayMode mode;
                if (SDL_GetDisplayMode(displayIndex, i, &mode) == 0) {
                    if (mode.w == desktopMode.w && mode.h == desktopMode.h) {
                        if (mode.refresh_rate > bestMode.refresh_rate) {
                            bestMode = mode;
                        }
                    }
                }
            }

            // Try to normalize values around our our standard refresh rates.
            // Some displays/OSes report values that are slightly off.
            if (bestMode.refresh_rate >= 58 && bestMode.refresh_rate <= 62) {
                monitorRefreshRates.append(60);
            }
            else if (bestMode.refresh_rate >= 28 && bestMode.refresh_rate <= 32) {
                monitorRefreshRates.append(30);
            }
            else {
                monitorRefreshRates.append(bestMode.refresh_rate);
            }
        }
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
