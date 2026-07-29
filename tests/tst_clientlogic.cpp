#include <QtTest>

#include "backend/hestiacapabilities.h"
#include "settings/streamingpreferences.h"
#include "streaming/hestianegotiation.h"
#include "streaming/video/statsdiagnostics.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QTranslator>

namespace {

VIDEO_STATS healthyStats()
{
    VIDEO_STATS stats {};
    stats.receivedFrames = 60;
    stats.decodedFrames = 60;
    stats.renderedFrames = 60;
    stats.totalFrames = 60;
    stats.receivedFps = 60;
    stats.decodedFps = 60;
    stats.renderedFps = 60;
    stats.totalDecodeTimeUs = 60ULL * 2000;
    stats.totalPacerTimeUs = 60ULL * 16000;
    stats.totalRenderTimeUs = 60ULL * 16600;
    stats.lastRtt = 8;
    stats.lastRttVariance = 1;
    return stats;
}

QJsonObject validCapabilities()
{
    return {
        {"ok", true},
        {"server_name", "Hermes"},
        {"base", "Apollo"},
        {"hestia_protocol", 1},
        {"min_client_protocol", 1},
        {"max_client_protocol", 1},
        {"server_version", "1.0.0"},
        {"compatibility", QJsonObject {
            {"gamestream", true},
            {"moonlight", true},
            {"sunshine", true},
        }},
        {"features", QJsonObject {
            {"virtual_display", true},
            {"multi_user_sessions", true},
            {"virtual_display_backend", QJsonArray {"hermes_kms"}},
            {"kde_kscreen", true},
            {"display_recovery", true},
            {"client_resolution_matching", true},
            {"client_fps_matching", true},
            {"hdr_mode_control", true},
            {"scale_factor", true},
            {"gamescope_session", true},
            {"server_commands", true},
            {"clipboard_sync", true},
            {"permission_system", true},
        }},
        {"limits", QJsonObject {
            {"max_width", 3840},
            {"max_height", 2160},
            {"max_fps", 120},
            {"supported_fps", QJsonArray {30, 60, 120}},
            {"supported_codecs", QJsonArray {"h264", "hevc", "av1"}},
        }},
    };
}

} // namespace

class ClientLogicTest : public QObject
{
    Q_OBJECT

private slots:
    void diagnosticsAcceptsNormalVsyncTime()
    {
        const Diagnostics::Diagnosis diagnosis = Diagnostics::diagnose(healthyStats(), 60);
        QCOMPARE(diagnosis.dominant, Diagnostics::BOTTLENECK_NONE);
        QVERIFY(!diagnosis.summary.isEmpty());
    }

    void diagnosticsClassifiesDecode()
    {
        VIDEO_STATS stats = healthyStats();
        stats.totalDecodeTimeUs = 60ULL * 12000;
        QCOMPARE(Diagnostics::diagnose(stats, 60).dominant,
                 Diagnostics::BOTTLENECK_DECODE);
    }

    void diagnosticsClassifiesNetworkLoss()
    {
        VIDEO_STATS stats = healthyStats();
        stats.networkDroppedFrames = 3;
        stats.totalFrames = 100;
        QCOMPARE(Diagnostics::diagnose(stats, 60).dominant,
                 Diagnostics::BOTTLENECK_NETWORK);
    }

    void diagnosticsAttributesJitteredPacerDropsToNetwork()
    {
        VIDEO_STATS stats = healthyStats();
        stats.pacerDroppedFrames = 3;
        stats.lastRttVariance = 15;
        QCOMPARE(Diagnostics::diagnose(stats, 60).dominant,
                 Diagnostics::BOTTLENECK_NETWORK);
    }

    void diagnosticsAttributesLocalBacklogToPresentation()
    {
        VIDEO_STATS stats = healthyStats();
        stats.totalPacerTimeUs = 60ULL * 40000;
        QCOMPARE(Diagnostics::diagnose(stats, 60).dominant,
                 Diagnostics::BOTTLENECK_RENDER);
    }

    void diagnosticsClassifiesHost()
    {
        VIDEO_STATS stats = healthyStats();
        stats.framesWithHostProcessingLatency = 60;
        stats.totalHostProcessingLatency = 60 * 400;
        QCOMPARE(Diagnostics::diagnose(stats, 60).dominant,
                 Diagnostics::BOTTLENECK_HOST);
    }

    void diagnosticsRequiresEnoughFrames()
    {
        VIDEO_STATS stats = healthyStats();
        stats.renderedFrames = 5;
        const Diagnostics::Diagnosis diagnosis = Diagnostics::diagnose(stats, 60);
        QCOMPARE(diagnosis.dominant, Diagnostics::BOTTLENECK_NONE);
        QVERIFY(diagnosis.summary.isEmpty());
    }

    void spikeHistoryCountsEvents()
    {
        Diagnostics::SpikeHistory history;
        Diagnostics::Diagnosis decode;
        decode.dominant = Diagnostics::BOTTLENECK_DECODE;
        Diagnostics::Diagnosis healthy;
        Diagnostics::Diagnosis network;
        network.dominant = Diagnostics::BOTTLENECK_NETWORK;
        network.summary = QStringLiteral("Network problem");
        network.keyMetric = QStringLiteral("15 ms");

        history.record(decode);
        history.record(decode);
        history.record(healthy);
        history.record(network);

        QCOMPARE(history.spikeCount(), 2);
        QVERIFY(history.summarize().contains(QStringLiteral("2")));
        const QString overlay = Diagnostics::formatOverlayText(network, history);
        QVERIFY(overlay.contains(QStringLiteral("Diagnosis:")));
        QVERIFY(overlay.contains(QStringLiteral("15 ms")));
        QVERIFY(overlay.contains(QStringLiteral("2")));
    }

    void portugueseDiagnosticsCatalog()
    {
        QTranslator translator;
        QVERIFY(translator.load(QStringLiteral("../app/languages/qml_pt_BR.qm")));
        QCoreApplication::installTranslator(&translator);

        QCOMPARE(QCoreApplication::translate("HestiaDiagnostics", "Diagnosis: %1"),
                 QStringLiteral("Diagnóstico: %1"));

        QCoreApplication::removeTranslator(&translator);
    }

    void qualityPresetUsesNativeMode()
    {
        const auto preset = StreamingPreferences::calculatePreset(
                StreamingPreferences::PRESET_QUALITY, 3840, 2160, 120, false);
        QCOMPARE(preset.width, 3840);
        QCOMPARE(preset.height, 2160);
        QCOMPARE(preset.fps, 120);
        QCOMPARE(preset.bitrateKbps,
                 qRound(StreamingPreferences::getDefaultBitrate(3840, 2160, 120, false) * 1.25));
    }

    void fastPresetFitsUltrawideBounds()
    {
        const auto preset = StreamingPreferences::calculatePreset(
                StreamingPreferences::PRESET_FAST, 3440, 1440, 144, false);
        QCOMPARE(preset.width, 1920);
        QVERIFY(preset.height <= 1080);
        QCOMPARE(preset.height % 2, 0);
        QCOMPARE(preset.fps, 60);
    }

    void batteryPresetPreservesHandheldAspect()
    {
        const auto preset = StreamingPreferences::calculatePreset(
                StreamingPreferences::PRESET_BATTERY, 1280, 800, 90, false);
        QCOMPARE(preset.width, 1152);
        QCOMPARE(preset.height, 720);
        QCOMPARE(preset.fps, 30);
    }

    void presetFallsBackToSensibleDisplayMode()
    {
        const auto preset = StreamingPreferences::calculatePreset(
                StreamingPreferences::PRESET_BALANCED, 0, 0, 0, false);
        QCOMPARE(preset.width, 1920);
        QCOMPARE(preset.height, 1080);
        QCOMPARE(preset.fps, 60);
    }

    void capabilitiesAcceptCompatibleFutureProtocol()
    {
        QJsonObject response = validCapabilities();
        response.insert("hestia_protocol", 2);
        response.insert("max_client_protocol", 2);
        response.insert("future_root_field", true);

        QJsonObject features = response.value("features").toObject();
        features.insert("virtual_display_backend",
                        QJsonArray {"hermes_kms", "future_kms"});
        features.insert("future_feature", true);
        response.insert("features", features);

        QJsonObject limits = response.value("limits").toObject();
        limits.insert("supported_codecs",
                      QJsonArray {"h264", "hevc", "av1", "future_codec"});
        response.insert("limits", limits);

        HestiaCapabilities capabilities;
        QString error;
        QVERIFY2(HestiaCapabilities::fromJson(response, &capabilities, &error),
                 qPrintable(error));
        QVERIFY(capabilities.supportsProtocolV1);
        QCOMPARE(capabilities.hestiaProtocol, 2);
        QCOMPARE(capabilities.maxClientProtocol, 2);
        QCOMPARE(capabilities.features.virtualDisplayBackend,
                 QStringList {"hermes_kms"});
        QCOMPARE(capabilities.limits.supportedCodecs,
                 QStringList({"h264", "hevc", "av1"}));
    }

    void capabilitiesRejectIncompatibleProtocolRange()
    {
        QJsonObject response = validCapabilities();
        response.insert("hestia_protocol", 2);
        response.insert("min_client_protocol", 2);
        response.insert("max_client_protocol", 3);

        HestiaCapabilities capabilities;
        QString error;
        QVERIFY(!HestiaCapabilities::fromJson(response, &capabilities, &error));
        QVERIFY(error.contains(QStringLiteral("protocol v1")));
    }

    void preflightTreatsUnknownStatusAsWarning()
    {
        const QJsonObject response {
            {"ok", true},
            {"preflight", QJsonObject {
                {"ready", true},
                {"checks", QJsonArray {
                    QJsonObject {
                        {"id", "future-check"},
                        {"status", "degraded"},
                        {"message", "Future warning"},
                    },
                }},
            }},
        };

        HestiaPreflight preflight;
        QString error;
        QVERIFY2(HestiaPreflight::fromDiagnosticsJson(response, &preflight, &error),
                 qPrintable(error));
        QVERIFY(preflight.valid);
        QCOMPARE(preflight.checks.size(), 1);
        QVERIFY(preflight.checks.first().isWarn());
    }

    void hermesNegotiationClampsModeAndPresetBitrate()
    {
        HestiaLimits limits;
        limits.maxWidth = 1920;
        limits.maxHeight = 1080;
        limits.maxFps = 60;
        limits.supportedFps = {30, 60};

        const HestiaNegotiation::StreamMode requested {
            3840,
            2160,
            120,
            90000,
        };
        const auto result = HestiaNegotiation::negotiateStreamMode(
                requested, limits, true, false);

        QCOMPARE(result.effective.width, 1920);
        QCOMPARE(result.effective.height, 1080);
        QCOMPARE(result.effective.fps, 60);
        QCOMPARE(result.effective.bitrateKbps,
                 StreamingPreferences::scaleBitrateForMode(
                         90000, 3840, 2160, 120, 1920, 1080, 60, false));
        QVERIFY(result.resolutionAdjusted);
        QVERIFY(result.fpsAdjusted);
        QVERIFY(result.bitrateAdjusted);
    }

    void hermesNegotiationPreservesCustomBitrate()
    {
        HestiaLimits limits;
        limits.maxWidth = 1920;
        limits.maxHeight = 1080;
        limits.maxFps = 60;
        limits.supportedFps = {60};

        const HestiaNegotiation::StreamMode requested {
            3840,
            2160,
            120,
            50000,
        };
        const auto result = HestiaNegotiation::negotiateStreamMode(
                requested, limits, false, false);
        QCOMPARE(result.effective.bitrateKbps, 50000);
        QVERIFY(!result.bitrateAdjusted);
    }
};

QTEST_GUILESS_MAIN(ClientLogicTest)

#include "tst_clientlogic.moc"
