/****************************************************************************
** Meta object code from reading C++ file 'streamingpreferences.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.18)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../settings/streamingpreferences.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'streamingpreferences.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.18. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_StreamingPreferences_t {
    QByteArrayData data[145];
    char stringdata0[2214];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_StreamingPreferences_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_StreamingPreferences_t qt_meta_stringdata_StreamingPreferences = {
    {
QT_MOC_LITERAL(0, 0, 20), // "StreamingPreferences"
QT_MOC_LITERAL(1, 21, 18), // "displayModeChanged"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 14), // "bitrateChanged"
QT_MOC_LITERAL(4, 56, 20), // "unlockBitrateChanged"
QT_MOC_LITERAL(5, 77, 24), // "autoAdjustBitrateChanged"
QT_MOC_LITERAL(6, 102, 18), // "enableVsyncChanged"
QT_MOC_LITERAL(7, 121, 24), // "gameOptimizationsChanged"
QT_MOC_LITERAL(8, 146, 22), // "playAudioOnHostChanged"
QT_MOC_LITERAL(9, 169, 22), // "multiControllerChanged"
QT_MOC_LITERAL(10, 192, 21), // "unsupportedFpsChanged"
QT_MOC_LITERAL(11, 214, 17), // "enableMdnsChanged"
QT_MOC_LITERAL(12, 232, 19), // "quitAppAfterChanged"
QT_MOC_LITERAL(13, 252, 24), // "absoluteMouseModeChanged"
QT_MOC_LITERAL(14, 277, 24), // "absoluteTouchModeChanged"
QT_MOC_LITERAL(15, 302, 18), // "audioConfigChanged"
QT_MOC_LITERAL(16, 321, 23), // "videoCodecConfigChanged"
QT_MOC_LITERAL(17, 345, 16), // "enableHdrChanged"
QT_MOC_LITERAL(18, 362, 24), // "hestiaPreferencesChanged"
QT_MOC_LITERAL(19, 387, 19), // "enableYUV444Changed"
QT_MOC_LITERAL(20, 407, 28), // "videoDecoderSelectionChanged"
QT_MOC_LITERAL(21, 436, 20), // "uiDisplayModeChanged"
QT_MOC_LITERAL(22, 457, 17), // "windowModeChanged"
QT_MOC_LITERAL(23, 475, 18), // "framePacingChanged"
QT_MOC_LITERAL(24, 494, 25), // "connectionWarningsChanged"
QT_MOC_LITERAL(25, 520, 28), // "configurationWarningsChanged"
QT_MOC_LITERAL(26, 549, 19), // "richPresenceChanged"
QT_MOC_LITERAL(27, 569, 19), // "gamepadMouseChanged"
QT_MOC_LITERAL(28, 589, 28), // "detectNetworkBlockingChanged"
QT_MOC_LITERAL(29, 618, 29), // "showPerformanceOverlayChanged"
QT_MOC_LITERAL(30, 648, 19), // "mouseButtonsChanged"
QT_MOC_LITERAL(31, 668, 22), // "muteOnFocusLossChanged"
QT_MOC_LITERAL(32, 691, 24), // "backgroundGamepadChanged"
QT_MOC_LITERAL(33, 716, 29), // "reverseScrollDirectionChanged"
QT_MOC_LITERAL(34, 746, 22), // "swapFaceButtonsChanged"
QT_MOC_LITERAL(35, 769, 25), // "captureSysKeysModeChanged"
QT_MOC_LITERAL(36, 795, 16), // "keepAwakeChanged"
QT_MOC_LITERAL(37, 812, 15), // "languageChanged"
QT_MOC_LITERAL(38, 828, 17), // "getDefaultBitrate"
QT_MOC_LITERAL(39, 846, 5), // "width"
QT_MOC_LITERAL(40, 852, 6), // "height"
QT_MOC_LITERAL(41, 859, 3), // "fps"
QT_MOC_LITERAL(42, 863, 6), // "yuv444"
QT_MOC_LITERAL(43, 870, 4), // "save"
QT_MOC_LITERAL(44, 875, 11), // "retranslate"
QT_MOC_LITERAL(45, 887, 11), // "bitrateKbps"
QT_MOC_LITERAL(46, 899, 13), // "unlockBitrate"
QT_MOC_LITERAL(47, 913, 17), // "autoAdjustBitrate"
QT_MOC_LITERAL(48, 931, 11), // "enableVsync"
QT_MOC_LITERAL(49, 943, 17), // "gameOptimizations"
QT_MOC_LITERAL(50, 961, 15), // "playAudioOnHost"
QT_MOC_LITERAL(51, 977, 15), // "multiController"
QT_MOC_LITERAL(52, 993, 10), // "enableMdns"
QT_MOC_LITERAL(53, 1004, 12), // "quitAppAfter"
QT_MOC_LITERAL(54, 1017, 17), // "absoluteMouseMode"
QT_MOC_LITERAL(55, 1035, 17), // "absoluteTouchMode"
QT_MOC_LITERAL(56, 1053, 11), // "framePacing"
QT_MOC_LITERAL(57, 1065, 18), // "connectionWarnings"
QT_MOC_LITERAL(58, 1084, 21), // "configurationWarnings"
QT_MOC_LITERAL(59, 1106, 12), // "richPresence"
QT_MOC_LITERAL(60, 1119, 12), // "gamepadMouse"
QT_MOC_LITERAL(61, 1132, 21), // "detectNetworkBlocking"
QT_MOC_LITERAL(62, 1154, 22), // "showPerformanceOverlay"
QT_MOC_LITERAL(63, 1177, 11), // "audioConfig"
QT_MOC_LITERAL(64, 1189, 11), // "AudioConfig"
QT_MOC_LITERAL(65, 1201, 16), // "videoCodecConfig"
QT_MOC_LITERAL(66, 1218, 16), // "VideoCodecConfig"
QT_MOC_LITERAL(67, 1235, 9), // "enableHdr"
QT_MOC_LITERAL(68, 1245, 20), // "hestiaVirtualDisplay"
QT_MOC_LITERAL(69, 1266, 19), // "hestiaClipboardSync"
QT_MOC_LITERAL(70, 1286, 17), // "hestiaScaleFactor"
QT_MOC_LITERAL(71, 1304, 16), // "hestiaLaunchMode"
QT_MOC_LITERAL(72, 1321, 16), // "HestiaLaunchMode"
QT_MOC_LITERAL(73, 1338, 12), // "enableYUV444"
QT_MOC_LITERAL(74, 1351, 21), // "videoDecoderSelection"
QT_MOC_LITERAL(75, 1373, 21), // "VideoDecoderSelection"
QT_MOC_LITERAL(76, 1395, 10), // "windowMode"
QT_MOC_LITERAL(77, 1406, 10), // "WindowMode"
QT_MOC_LITERAL(78, 1417, 25), // "recommendedFullScreenMode"
QT_MOC_LITERAL(79, 1443, 13), // "uiDisplayMode"
QT_MOC_LITERAL(80, 1457, 13), // "UIDisplayMode"
QT_MOC_LITERAL(81, 1471, 16), // "swapMouseButtons"
QT_MOC_LITERAL(82, 1488, 15), // "muteOnFocusLoss"
QT_MOC_LITERAL(83, 1504, 17), // "backgroundGamepad"
QT_MOC_LITERAL(84, 1522, 22), // "reverseScrollDirection"
QT_MOC_LITERAL(85, 1545, 15), // "swapFaceButtons"
QT_MOC_LITERAL(86, 1561, 9), // "keepAwake"
QT_MOC_LITERAL(87, 1571, 18), // "captureSysKeysMode"
QT_MOC_LITERAL(88, 1590, 18), // "CaptureSysKeysMode"
QT_MOC_LITERAL(89, 1609, 8), // "language"
QT_MOC_LITERAL(90, 1618, 8), // "Language"
QT_MOC_LITERAL(91, 1627, 9), // "AC_STEREO"
QT_MOC_LITERAL(92, 1637, 14), // "AC_51_SURROUND"
QT_MOC_LITERAL(93, 1652, 14), // "AC_71_SURROUND"
QT_MOC_LITERAL(94, 1667, 8), // "VCC_AUTO"
QT_MOC_LITERAL(95, 1676, 14), // "VCC_FORCE_H264"
QT_MOC_LITERAL(96, 1691, 14), // "VCC_FORCE_HEVC"
QT_MOC_LITERAL(97, 1706, 29), // "VCC_FORCE_HEVC_HDR_DEPRECATED"
QT_MOC_LITERAL(98, 1736, 13), // "VCC_FORCE_AV1"
QT_MOC_LITERAL(99, 1750, 8), // "VDS_AUTO"
QT_MOC_LITERAL(100, 1759, 18), // "VDS_FORCE_HARDWARE"
QT_MOC_LITERAL(101, 1778, 18), // "VDS_FORCE_SOFTWARE"
QT_MOC_LITERAL(102, 1797, 10), // "HLM_NORMAL"
QT_MOC_LITERAL(103, 1808, 13), // "HLM_GAMESCOPE"
QT_MOC_LITERAL(104, 1822, 13), // "WM_FULLSCREEN"
QT_MOC_LITERAL(105, 1836, 21), // "WM_FULLSCREEN_DESKTOP"
QT_MOC_LITERAL(106, 1858, 11), // "WM_WINDOWED"
QT_MOC_LITERAL(107, 1870, 11), // "UI_WINDOWED"
QT_MOC_LITERAL(108, 1882, 12), // "UI_MAXIMIZED"
QT_MOC_LITERAL(109, 1895, 13), // "UI_FULLSCREEN"
QT_MOC_LITERAL(110, 1909, 9), // "LANG_AUTO"
QT_MOC_LITERAL(111, 1919, 7), // "LANG_EN"
QT_MOC_LITERAL(112, 1927, 7), // "LANG_FR"
QT_MOC_LITERAL(113, 1935, 10), // "LANG_ZH_CN"
QT_MOC_LITERAL(114, 1946, 7), // "LANG_DE"
QT_MOC_LITERAL(115, 1954, 10), // "LANG_NB_NO"
QT_MOC_LITERAL(116, 1965, 7), // "LANG_RU"
QT_MOC_LITERAL(117, 1973, 7), // "LANG_ES"
QT_MOC_LITERAL(118, 1981, 7), // "LANG_JA"
QT_MOC_LITERAL(119, 1989, 7), // "LANG_VI"
QT_MOC_LITERAL(120, 1997, 7), // "LANG_TH"
QT_MOC_LITERAL(121, 2005, 7), // "LANG_KO"
QT_MOC_LITERAL(122, 2013, 7), // "LANG_HU"
QT_MOC_LITERAL(123, 2021, 7), // "LANG_NL"
QT_MOC_LITERAL(124, 2029, 7), // "LANG_SV"
QT_MOC_LITERAL(125, 2037, 7), // "LANG_TR"
QT_MOC_LITERAL(126, 2045, 7), // "LANG_UK"
QT_MOC_LITERAL(127, 2053, 10), // "LANG_ZH_TW"
QT_MOC_LITERAL(128, 2064, 7), // "LANG_PT"
QT_MOC_LITERAL(129, 2072, 10), // "LANG_PT_BR"
QT_MOC_LITERAL(130, 2083, 7), // "LANG_EL"
QT_MOC_LITERAL(131, 2091, 7), // "LANG_IT"
QT_MOC_LITERAL(132, 2099, 7), // "LANG_HI"
QT_MOC_LITERAL(133, 2107, 7), // "LANG_PL"
QT_MOC_LITERAL(134, 2115, 7), // "LANG_CS"
QT_MOC_LITERAL(135, 2123, 7), // "LANG_HE"
QT_MOC_LITERAL(136, 2131, 8), // "LANG_CKB"
QT_MOC_LITERAL(137, 2140, 7), // "LANG_LT"
QT_MOC_LITERAL(138, 2148, 7), // "LANG_ET"
QT_MOC_LITERAL(139, 2156, 7), // "LANG_BG"
QT_MOC_LITERAL(140, 2164, 7), // "LANG_EO"
QT_MOC_LITERAL(141, 2172, 7), // "LANG_TA"
QT_MOC_LITERAL(142, 2180, 7), // "CSK_OFF"
QT_MOC_LITERAL(143, 2188, 14), // "CSK_FULLSCREEN"
QT_MOC_LITERAL(144, 2203, 10) // "CSK_ALWAYS"

    },
    "StreamingPreferences\0displayModeChanged\0"
    "\0bitrateChanged\0unlockBitrateChanged\0"
    "autoAdjustBitrateChanged\0enableVsyncChanged\0"
    "gameOptimizationsChanged\0"
    "playAudioOnHostChanged\0multiControllerChanged\0"
    "unsupportedFpsChanged\0enableMdnsChanged\0"
    "quitAppAfterChanged\0absoluteMouseModeChanged\0"
    "absoluteTouchModeChanged\0audioConfigChanged\0"
    "videoCodecConfigChanged\0enableHdrChanged\0"
    "hestiaPreferencesChanged\0enableYUV444Changed\0"
    "videoDecoderSelectionChanged\0"
    "uiDisplayModeChanged\0windowModeChanged\0"
    "framePacingChanged\0connectionWarningsChanged\0"
    "configurationWarningsChanged\0"
    "richPresenceChanged\0gamepadMouseChanged\0"
    "detectNetworkBlockingChanged\0"
    "showPerformanceOverlayChanged\0"
    "mouseButtonsChanged\0muteOnFocusLossChanged\0"
    "backgroundGamepadChanged\0"
    "reverseScrollDirectionChanged\0"
    "swapFaceButtonsChanged\0captureSysKeysModeChanged\0"
    "keepAwakeChanged\0languageChanged\0"
    "getDefaultBitrate\0width\0height\0fps\0"
    "yuv444\0save\0retranslate\0bitrateKbps\0"
    "unlockBitrate\0autoAdjustBitrate\0"
    "enableVsync\0gameOptimizations\0"
    "playAudioOnHost\0multiController\0"
    "enableMdns\0quitAppAfter\0absoluteMouseMode\0"
    "absoluteTouchMode\0framePacing\0"
    "connectionWarnings\0configurationWarnings\0"
    "richPresence\0gamepadMouse\0"
    "detectNetworkBlocking\0showPerformanceOverlay\0"
    "audioConfig\0AudioConfig\0videoCodecConfig\0"
    "VideoCodecConfig\0enableHdr\0"
    "hestiaVirtualDisplay\0hestiaClipboardSync\0"
    "hestiaScaleFactor\0hestiaLaunchMode\0"
    "HestiaLaunchMode\0enableYUV444\0"
    "videoDecoderSelection\0VideoDecoderSelection\0"
    "windowMode\0WindowMode\0recommendedFullScreenMode\0"
    "uiDisplayMode\0UIDisplayMode\0"
    "swapMouseButtons\0muteOnFocusLoss\0"
    "backgroundGamepad\0reverseScrollDirection\0"
    "swapFaceButtons\0keepAwake\0captureSysKeysMode\0"
    "CaptureSysKeysMode\0language\0Language\0"
    "AC_STEREO\0AC_51_SURROUND\0AC_71_SURROUND\0"
    "VCC_AUTO\0VCC_FORCE_H264\0VCC_FORCE_HEVC\0"
    "VCC_FORCE_HEVC_HDR_DEPRECATED\0"
    "VCC_FORCE_AV1\0VDS_AUTO\0VDS_FORCE_HARDWARE\0"
    "VDS_FORCE_SOFTWARE\0HLM_NORMAL\0"
    "HLM_GAMESCOPE\0WM_FULLSCREEN\0"
    "WM_FULLSCREEN_DESKTOP\0WM_WINDOWED\0"
    "UI_WINDOWED\0UI_MAXIMIZED\0UI_FULLSCREEN\0"
    "LANG_AUTO\0LANG_EN\0LANG_FR\0LANG_ZH_CN\0"
    "LANG_DE\0LANG_NB_NO\0LANG_RU\0LANG_ES\0"
    "LANG_JA\0LANG_VI\0LANG_TH\0LANG_KO\0LANG_HU\0"
    "LANG_NL\0LANG_SV\0LANG_TR\0LANG_UK\0"
    "LANG_ZH_TW\0LANG_PT\0LANG_PT_BR\0LANG_EL\0"
    "LANG_IT\0LANG_HI\0LANG_PL\0LANG_CS\0LANG_HE\0"
    "LANG_CKB\0LANG_LT\0LANG_ET\0LANG_BG\0"
    "LANG_EO\0LANG_TA\0CSK_OFF\0CSK_FULLSCREEN\0"
    "CSK_ALWAYS"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_StreamingPreferences[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      39,   14, // methods
      41,  256, // properties
       8,  420, // enums/sets
       0,    0, // constructors
       0,       // flags
      36,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  209,    2, 0x06 /* Public */,
       3,    0,  210,    2, 0x06 /* Public */,
       4,    0,  211,    2, 0x06 /* Public */,
       5,    0,  212,    2, 0x06 /* Public */,
       6,    0,  213,    2, 0x06 /* Public */,
       7,    0,  214,    2, 0x06 /* Public */,
       8,    0,  215,    2, 0x06 /* Public */,
       9,    0,  216,    2, 0x06 /* Public */,
      10,    0,  217,    2, 0x06 /* Public */,
      11,    0,  218,    2, 0x06 /* Public */,
      12,    0,  219,    2, 0x06 /* Public */,
      13,    0,  220,    2, 0x06 /* Public */,
      14,    0,  221,    2, 0x06 /* Public */,
      15,    0,  222,    2, 0x06 /* Public */,
      16,    0,  223,    2, 0x06 /* Public */,
      17,    0,  224,    2, 0x06 /* Public */,
      18,    0,  225,    2, 0x06 /* Public */,
      19,    0,  226,    2, 0x06 /* Public */,
      20,    0,  227,    2, 0x06 /* Public */,
      21,    0,  228,    2, 0x06 /* Public */,
      22,    0,  229,    2, 0x06 /* Public */,
      23,    0,  230,    2, 0x06 /* Public */,
      24,    0,  231,    2, 0x06 /* Public */,
      25,    0,  232,    2, 0x06 /* Public */,
      26,    0,  233,    2, 0x06 /* Public */,
      27,    0,  234,    2, 0x06 /* Public */,
      28,    0,  235,    2, 0x06 /* Public */,
      29,    0,  236,    2, 0x06 /* Public */,
      30,    0,  237,    2, 0x06 /* Public */,
      31,    0,  238,    2, 0x06 /* Public */,
      32,    0,  239,    2, 0x06 /* Public */,
      33,    0,  240,    2, 0x06 /* Public */,
      34,    0,  241,    2, 0x06 /* Public */,
      35,    0,  242,    2, 0x06 /* Public */,
      36,    0,  243,    2, 0x06 /* Public */,
      37,    0,  244,    2, 0x06 /* Public */,

 // methods: name, argc, parameters, tag, flags
      38,    4,  245,    2, 0x02 /* Public */,
      43,    0,  254,    2, 0x02 /* Public */,
      44,    0,  255,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Bool,   39,   40,   41,   42,
    QMetaType::Void,
    QMetaType::Bool,

 // properties: name, type, flags
      39, QMetaType::Int, 0x00495003,
      40, QMetaType::Int, 0x00495003,
      41, QMetaType::Int, 0x00495003,
      45, QMetaType::Int, 0x00495003,
      46, QMetaType::Bool, 0x00495003,
      47, QMetaType::Bool, 0x00495003,
      48, QMetaType::Bool, 0x00495003,
      49, QMetaType::Bool, 0x00495003,
      50, QMetaType::Bool, 0x00495003,
      51, QMetaType::Bool, 0x00495003,
      52, QMetaType::Bool, 0x00495003,
      53, QMetaType::Bool, 0x00495003,
      54, QMetaType::Bool, 0x00495003,
      55, QMetaType::Bool, 0x00495003,
      56, QMetaType::Bool, 0x00495003,
      57, QMetaType::Bool, 0x00495003,
      58, QMetaType::Bool, 0x00495003,
      59, QMetaType::Bool, 0x00495003,
      60, QMetaType::Bool, 0x00495003,
      61, QMetaType::Bool, 0x00495003,
      62, QMetaType::Bool, 0x00495003,
      63, 0x80000000 | 64, 0x0049500b,
      65, 0x80000000 | 66, 0x0049500b,
      67, QMetaType::Bool, 0x00495003,
      68, QMetaType::Bool, 0x00495003,
      69, QMetaType::Bool, 0x00495003,
      70, QMetaType::Int, 0x00495003,
      71, 0x80000000 | 72, 0x0049500b,
      73, QMetaType::Bool, 0x00495003,
      74, 0x80000000 | 75, 0x0049500b,
      76, 0x80000000 | 77, 0x0049500b,
      78, 0x80000000 | 77, 0x00095409,
      79, 0x80000000 | 80, 0x0049500b,
      81, QMetaType::Bool, 0x00495003,
      82, QMetaType::Bool, 0x00495003,
      83, QMetaType::Bool, 0x00495003,
      84, QMetaType::Bool, 0x00495003,
      85, QMetaType::Bool, 0x00495003,
      86, QMetaType::Bool, 0x00495003,
      87, 0x80000000 | 88, 0x0049500b,
      89, 0x80000000 | 90, 0x0049500b,

 // properties: notify_signal_id
       0,
       0,
       0,
       1,
       2,
       3,
       4,
       5,
       6,
       7,
       9,
      10,
      11,
      12,
      21,
      22,
      23,
      24,
      25,
      26,
      27,
      13,
      14,
      15,
      16,
      16,
      16,
      16,
      17,
      18,
      20,
       0,
      19,
      28,
      29,
      30,
      31,
      32,
      34,
      33,
      35,

 // enums: name, alias, flags, count, data
      64,   64, 0x0,    3,  460,
      66,   66, 0x0,    5,  466,
      75,   75, 0x0,    3,  476,
      72,   72, 0x0,    2,  482,
      77,   77, 0x0,    3,  486,
      80,   80, 0x0,    3,  492,
      90,   90, 0x0,   32,  498,
      88,   88, 0x0,    3,  562,

 // enum data: key, value
      91, uint(StreamingPreferences::AC_STEREO),
      92, uint(StreamingPreferences::AC_51_SURROUND),
      93, uint(StreamingPreferences::AC_71_SURROUND),
      94, uint(StreamingPreferences::VCC_AUTO),
      95, uint(StreamingPreferences::VCC_FORCE_H264),
      96, uint(StreamingPreferences::VCC_FORCE_HEVC),
      97, uint(StreamingPreferences::VCC_FORCE_HEVC_HDR_DEPRECATED),
      98, uint(StreamingPreferences::VCC_FORCE_AV1),
      99, uint(StreamingPreferences::VDS_AUTO),
     100, uint(StreamingPreferences::VDS_FORCE_HARDWARE),
     101, uint(StreamingPreferences::VDS_FORCE_SOFTWARE),
     102, uint(StreamingPreferences::HLM_NORMAL),
     103, uint(StreamingPreferences::HLM_GAMESCOPE),
     104, uint(StreamingPreferences::WM_FULLSCREEN),
     105, uint(StreamingPreferences::WM_FULLSCREEN_DESKTOP),
     106, uint(StreamingPreferences::WM_WINDOWED),
     107, uint(StreamingPreferences::UI_WINDOWED),
     108, uint(StreamingPreferences::UI_MAXIMIZED),
     109, uint(StreamingPreferences::UI_FULLSCREEN),
     110, uint(StreamingPreferences::LANG_AUTO),
     111, uint(StreamingPreferences::LANG_EN),
     112, uint(StreamingPreferences::LANG_FR),
     113, uint(StreamingPreferences::LANG_ZH_CN),
     114, uint(StreamingPreferences::LANG_DE),
     115, uint(StreamingPreferences::LANG_NB_NO),
     116, uint(StreamingPreferences::LANG_RU),
     117, uint(StreamingPreferences::LANG_ES),
     118, uint(StreamingPreferences::LANG_JA),
     119, uint(StreamingPreferences::LANG_VI),
     120, uint(StreamingPreferences::LANG_TH),
     121, uint(StreamingPreferences::LANG_KO),
     122, uint(StreamingPreferences::LANG_HU),
     123, uint(StreamingPreferences::LANG_NL),
     124, uint(StreamingPreferences::LANG_SV),
     125, uint(StreamingPreferences::LANG_TR),
     126, uint(StreamingPreferences::LANG_UK),
     127, uint(StreamingPreferences::LANG_ZH_TW),
     128, uint(StreamingPreferences::LANG_PT),
     129, uint(StreamingPreferences::LANG_PT_BR),
     130, uint(StreamingPreferences::LANG_EL),
     131, uint(StreamingPreferences::LANG_IT),
     132, uint(StreamingPreferences::LANG_HI),
     133, uint(StreamingPreferences::LANG_PL),
     134, uint(StreamingPreferences::LANG_CS),
     135, uint(StreamingPreferences::LANG_HE),
     136, uint(StreamingPreferences::LANG_CKB),
     137, uint(StreamingPreferences::LANG_LT),
     138, uint(StreamingPreferences::LANG_ET),
     139, uint(StreamingPreferences::LANG_BG),
     140, uint(StreamingPreferences::LANG_EO),
     141, uint(StreamingPreferences::LANG_TA),
     142, uint(StreamingPreferences::CSK_OFF),
     143, uint(StreamingPreferences::CSK_FULLSCREEN),
     144, uint(StreamingPreferences::CSK_ALWAYS),

       0        // eod
};

void StreamingPreferences::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<StreamingPreferences *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->displayModeChanged(); break;
        case 1: _t->bitrateChanged(); break;
        case 2: _t->unlockBitrateChanged(); break;
        case 3: _t->autoAdjustBitrateChanged(); break;
        case 4: _t->enableVsyncChanged(); break;
        case 5: _t->gameOptimizationsChanged(); break;
        case 6: _t->playAudioOnHostChanged(); break;
        case 7: _t->multiControllerChanged(); break;
        case 8: _t->unsupportedFpsChanged(); break;
        case 9: _t->enableMdnsChanged(); break;
        case 10: _t->quitAppAfterChanged(); break;
        case 11: _t->absoluteMouseModeChanged(); break;
        case 12: _t->absoluteTouchModeChanged(); break;
        case 13: _t->audioConfigChanged(); break;
        case 14: _t->videoCodecConfigChanged(); break;
        case 15: _t->enableHdrChanged(); break;
        case 16: _t->hestiaPreferencesChanged(); break;
        case 17: _t->enableYUV444Changed(); break;
        case 18: _t->videoDecoderSelectionChanged(); break;
        case 19: _t->uiDisplayModeChanged(); break;
        case 20: _t->windowModeChanged(); break;
        case 21: _t->framePacingChanged(); break;
        case 22: _t->connectionWarningsChanged(); break;
        case 23: _t->configurationWarningsChanged(); break;
        case 24: _t->richPresenceChanged(); break;
        case 25: _t->gamepadMouseChanged(); break;
        case 26: _t->detectNetworkBlockingChanged(); break;
        case 27: _t->showPerformanceOverlayChanged(); break;
        case 28: _t->mouseButtonsChanged(); break;
        case 29: _t->muteOnFocusLossChanged(); break;
        case 30: _t->backgroundGamepadChanged(); break;
        case 31: _t->reverseScrollDirectionChanged(); break;
        case 32: _t->swapFaceButtonsChanged(); break;
        case 33: _t->captureSysKeysModeChanged(); break;
        case 34: _t->keepAwakeChanged(); break;
        case 35: _t->languageChanged(); break;
        case 36: { int _r = _t->getDefaultBitrate((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 37: _t->save(); break;
        case 38: { bool _r = _t->retranslate();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::displayModeChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::bitrateChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::unlockBitrateChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::autoAdjustBitrateChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::enableVsyncChanged)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::gameOptimizationsChanged)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::playAudioOnHostChanged)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::multiControllerChanged)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::unsupportedFpsChanged)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::enableMdnsChanged)) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::quitAppAfterChanged)) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::absoluteMouseModeChanged)) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::absoluteTouchModeChanged)) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::audioConfigChanged)) {
                *result = 13;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::videoCodecConfigChanged)) {
                *result = 14;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::enableHdrChanged)) {
                *result = 15;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::hestiaPreferencesChanged)) {
                *result = 16;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::enableYUV444Changed)) {
                *result = 17;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::videoDecoderSelectionChanged)) {
                *result = 18;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::uiDisplayModeChanged)) {
                *result = 19;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::windowModeChanged)) {
                *result = 20;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::framePacingChanged)) {
                *result = 21;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::connectionWarningsChanged)) {
                *result = 22;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::configurationWarningsChanged)) {
                *result = 23;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::richPresenceChanged)) {
                *result = 24;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::gamepadMouseChanged)) {
                *result = 25;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::detectNetworkBlockingChanged)) {
                *result = 26;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::showPerformanceOverlayChanged)) {
                *result = 27;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::mouseButtonsChanged)) {
                *result = 28;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::muteOnFocusLossChanged)) {
                *result = 29;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::backgroundGamepadChanged)) {
                *result = 30;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::reverseScrollDirectionChanged)) {
                *result = 31;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::swapFaceButtonsChanged)) {
                *result = 32;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::captureSysKeysModeChanged)) {
                *result = 33;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::keepAwakeChanged)) {
                *result = 34;
                return;
            }
        }
        {
            using _t = void (StreamingPreferences::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamingPreferences::languageChanged)) {
                *result = 35;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<StreamingPreferences *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = _t->width; break;
        case 1: *reinterpret_cast< int*>(_v) = _t->height; break;
        case 2: *reinterpret_cast< int*>(_v) = _t->fps; break;
        case 3: *reinterpret_cast< int*>(_v) = _t->bitrateKbps; break;
        case 4: *reinterpret_cast< bool*>(_v) = _t->unlockBitrate; break;
        case 5: *reinterpret_cast< bool*>(_v) = _t->autoAdjustBitrate; break;
        case 6: *reinterpret_cast< bool*>(_v) = _t->enableVsync; break;
        case 7: *reinterpret_cast< bool*>(_v) = _t->gameOptimizations; break;
        case 8: *reinterpret_cast< bool*>(_v) = _t->playAudioOnHost; break;
        case 9: *reinterpret_cast< bool*>(_v) = _t->multiController; break;
        case 10: *reinterpret_cast< bool*>(_v) = _t->enableMdns; break;
        case 11: *reinterpret_cast< bool*>(_v) = _t->quitAppAfter; break;
        case 12: *reinterpret_cast< bool*>(_v) = _t->absoluteMouseMode; break;
        case 13: *reinterpret_cast< bool*>(_v) = _t->absoluteTouchMode; break;
        case 14: *reinterpret_cast< bool*>(_v) = _t->framePacing; break;
        case 15: *reinterpret_cast< bool*>(_v) = _t->connectionWarnings; break;
        case 16: *reinterpret_cast< bool*>(_v) = _t->configurationWarnings; break;
        case 17: *reinterpret_cast< bool*>(_v) = _t->richPresence; break;
        case 18: *reinterpret_cast< bool*>(_v) = _t->gamepadMouse; break;
        case 19: *reinterpret_cast< bool*>(_v) = _t->detectNetworkBlocking; break;
        case 20: *reinterpret_cast< bool*>(_v) = _t->showPerformanceOverlay; break;
        case 21: *reinterpret_cast< AudioConfig*>(_v) = _t->audioConfig; break;
        case 22: *reinterpret_cast< VideoCodecConfig*>(_v) = _t->videoCodecConfig; break;
        case 23: *reinterpret_cast< bool*>(_v) = _t->enableHdr; break;
        case 24: *reinterpret_cast< bool*>(_v) = _t->hestiaVirtualDisplay; break;
        case 25: *reinterpret_cast< bool*>(_v) = _t->hestiaClipboardSync; break;
        case 26: *reinterpret_cast< int*>(_v) = _t->hestiaScaleFactor; break;
        case 27: *reinterpret_cast< HestiaLaunchMode*>(_v) = _t->hestiaLaunchMode; break;
        case 28: *reinterpret_cast< bool*>(_v) = _t->enableYUV444; break;
        case 29: *reinterpret_cast< VideoDecoderSelection*>(_v) = _t->videoDecoderSelection; break;
        case 30: *reinterpret_cast< WindowMode*>(_v) = _t->windowMode; break;
        case 31: *reinterpret_cast< WindowMode*>(_v) = _t->recommendedFullScreenMode; break;
        case 32: *reinterpret_cast< UIDisplayMode*>(_v) = _t->uiDisplayMode; break;
        case 33: *reinterpret_cast< bool*>(_v) = _t->swapMouseButtons; break;
        case 34: *reinterpret_cast< bool*>(_v) = _t->muteOnFocusLoss; break;
        case 35: *reinterpret_cast< bool*>(_v) = _t->backgroundGamepad; break;
        case 36: *reinterpret_cast< bool*>(_v) = _t->reverseScrollDirection; break;
        case 37: *reinterpret_cast< bool*>(_v) = _t->swapFaceButtons; break;
        case 38: *reinterpret_cast< bool*>(_v) = _t->keepAwake; break;
        case 39: *reinterpret_cast< CaptureSysKeysMode*>(_v) = _t->captureSysKeysMode; break;
        case 40: *reinterpret_cast< Language*>(_v) = _t->language; break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<StreamingPreferences *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0:
            if (_t->width != *reinterpret_cast< int*>(_v)) {
                _t->width = *reinterpret_cast< int*>(_v);
                Q_EMIT _t->displayModeChanged();
            }
            break;
        case 1:
            if (_t->height != *reinterpret_cast< int*>(_v)) {
                _t->height = *reinterpret_cast< int*>(_v);
                Q_EMIT _t->displayModeChanged();
            }
            break;
        case 2:
            if (_t->fps != *reinterpret_cast< int*>(_v)) {
                _t->fps = *reinterpret_cast< int*>(_v);
                Q_EMIT _t->displayModeChanged();
            }
            break;
        case 3:
            if (_t->bitrateKbps != *reinterpret_cast< int*>(_v)) {
                _t->bitrateKbps = *reinterpret_cast< int*>(_v);
                Q_EMIT _t->bitrateChanged();
            }
            break;
        case 4:
            if (_t->unlockBitrate != *reinterpret_cast< bool*>(_v)) {
                _t->unlockBitrate = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->unlockBitrateChanged();
            }
            break;
        case 5:
            if (_t->autoAdjustBitrate != *reinterpret_cast< bool*>(_v)) {
                _t->autoAdjustBitrate = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->autoAdjustBitrateChanged();
            }
            break;
        case 6:
            if (_t->enableVsync != *reinterpret_cast< bool*>(_v)) {
                _t->enableVsync = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->enableVsyncChanged();
            }
            break;
        case 7:
            if (_t->gameOptimizations != *reinterpret_cast< bool*>(_v)) {
                _t->gameOptimizations = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->gameOptimizationsChanged();
            }
            break;
        case 8:
            if (_t->playAudioOnHost != *reinterpret_cast< bool*>(_v)) {
                _t->playAudioOnHost = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->playAudioOnHostChanged();
            }
            break;
        case 9:
            if (_t->multiController != *reinterpret_cast< bool*>(_v)) {
                _t->multiController = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->multiControllerChanged();
            }
            break;
        case 10:
            if (_t->enableMdns != *reinterpret_cast< bool*>(_v)) {
                _t->enableMdns = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->enableMdnsChanged();
            }
            break;
        case 11:
            if (_t->quitAppAfter != *reinterpret_cast< bool*>(_v)) {
                _t->quitAppAfter = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->quitAppAfterChanged();
            }
            break;
        case 12:
            if (_t->absoluteMouseMode != *reinterpret_cast< bool*>(_v)) {
                _t->absoluteMouseMode = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->absoluteMouseModeChanged();
            }
            break;
        case 13:
            if (_t->absoluteTouchMode != *reinterpret_cast< bool*>(_v)) {
                _t->absoluteTouchMode = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->absoluteTouchModeChanged();
            }
            break;
        case 14:
            if (_t->framePacing != *reinterpret_cast< bool*>(_v)) {
                _t->framePacing = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->framePacingChanged();
            }
            break;
        case 15:
            if (_t->connectionWarnings != *reinterpret_cast< bool*>(_v)) {
                _t->connectionWarnings = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->connectionWarningsChanged();
            }
            break;
        case 16:
            if (_t->configurationWarnings != *reinterpret_cast< bool*>(_v)) {
                _t->configurationWarnings = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->configurationWarningsChanged();
            }
            break;
        case 17:
            if (_t->richPresence != *reinterpret_cast< bool*>(_v)) {
                _t->richPresence = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->richPresenceChanged();
            }
            break;
        case 18:
            if (_t->gamepadMouse != *reinterpret_cast< bool*>(_v)) {
                _t->gamepadMouse = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->gamepadMouseChanged();
            }
            break;
        case 19:
            if (_t->detectNetworkBlocking != *reinterpret_cast< bool*>(_v)) {
                _t->detectNetworkBlocking = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->detectNetworkBlockingChanged();
            }
            break;
        case 20:
            if (_t->showPerformanceOverlay != *reinterpret_cast< bool*>(_v)) {
                _t->showPerformanceOverlay = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->showPerformanceOverlayChanged();
            }
            break;
        case 21:
            if (_t->audioConfig != *reinterpret_cast< AudioConfig*>(_v)) {
                _t->audioConfig = *reinterpret_cast< AudioConfig*>(_v);
                Q_EMIT _t->audioConfigChanged();
            }
            break;
        case 22:
            if (_t->videoCodecConfig != *reinterpret_cast< VideoCodecConfig*>(_v)) {
                _t->videoCodecConfig = *reinterpret_cast< VideoCodecConfig*>(_v);
                Q_EMIT _t->videoCodecConfigChanged();
            }
            break;
        case 23:
            if (_t->enableHdr != *reinterpret_cast< bool*>(_v)) {
                _t->enableHdr = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->enableHdrChanged();
            }
            break;
        case 24:
            if (_t->hestiaVirtualDisplay != *reinterpret_cast< bool*>(_v)) {
                _t->hestiaVirtualDisplay = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->hestiaPreferencesChanged();
            }
            break;
        case 25:
            if (_t->hestiaClipboardSync != *reinterpret_cast< bool*>(_v)) {
                _t->hestiaClipboardSync = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->hestiaPreferencesChanged();
            }
            break;
        case 26:
            if (_t->hestiaScaleFactor != *reinterpret_cast< int*>(_v)) {
                _t->hestiaScaleFactor = *reinterpret_cast< int*>(_v);
                Q_EMIT _t->hestiaPreferencesChanged();
            }
            break;
        case 27:
            if (_t->hestiaLaunchMode != *reinterpret_cast< HestiaLaunchMode*>(_v)) {
                _t->hestiaLaunchMode = *reinterpret_cast< HestiaLaunchMode*>(_v);
                Q_EMIT _t->hestiaPreferencesChanged();
            }
            break;
        case 28:
            if (_t->enableYUV444 != *reinterpret_cast< bool*>(_v)) {
                _t->enableYUV444 = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->enableYUV444Changed();
            }
            break;
        case 29:
            if (_t->videoDecoderSelection != *reinterpret_cast< VideoDecoderSelection*>(_v)) {
                _t->videoDecoderSelection = *reinterpret_cast< VideoDecoderSelection*>(_v);
                Q_EMIT _t->videoDecoderSelectionChanged();
            }
            break;
        case 30:
            if (_t->windowMode != *reinterpret_cast< WindowMode*>(_v)) {
                _t->windowMode = *reinterpret_cast< WindowMode*>(_v);
                Q_EMIT _t->windowModeChanged();
            }
            break;
        case 32:
            if (_t->uiDisplayMode != *reinterpret_cast< UIDisplayMode*>(_v)) {
                _t->uiDisplayMode = *reinterpret_cast< UIDisplayMode*>(_v);
                Q_EMIT _t->uiDisplayModeChanged();
            }
            break;
        case 33:
            if (_t->swapMouseButtons != *reinterpret_cast< bool*>(_v)) {
                _t->swapMouseButtons = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->mouseButtonsChanged();
            }
            break;
        case 34:
            if (_t->muteOnFocusLoss != *reinterpret_cast< bool*>(_v)) {
                _t->muteOnFocusLoss = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->muteOnFocusLossChanged();
            }
            break;
        case 35:
            if (_t->backgroundGamepad != *reinterpret_cast< bool*>(_v)) {
                _t->backgroundGamepad = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->backgroundGamepadChanged();
            }
            break;
        case 36:
            if (_t->reverseScrollDirection != *reinterpret_cast< bool*>(_v)) {
                _t->reverseScrollDirection = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->reverseScrollDirectionChanged();
            }
            break;
        case 37:
            if (_t->swapFaceButtons != *reinterpret_cast< bool*>(_v)) {
                _t->swapFaceButtons = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->swapFaceButtonsChanged();
            }
            break;
        case 38:
            if (_t->keepAwake != *reinterpret_cast< bool*>(_v)) {
                _t->keepAwake = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->keepAwakeChanged();
            }
            break;
        case 39:
            if (_t->captureSysKeysMode != *reinterpret_cast< CaptureSysKeysMode*>(_v)) {
                _t->captureSysKeysMode = *reinterpret_cast< CaptureSysKeysMode*>(_v);
                Q_EMIT _t->captureSysKeysModeChanged();
            }
            break;
        case 40:
            if (_t->language != *reinterpret_cast< Language*>(_v)) {
                _t->language = *reinterpret_cast< Language*>(_v);
                Q_EMIT _t->languageChanged();
            }
            break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject StreamingPreferences::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_StreamingPreferences.data,
    qt_meta_data_StreamingPreferences,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *StreamingPreferences::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *StreamingPreferences::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_StreamingPreferences.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int StreamingPreferences::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 39)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 39;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 39)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 39;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 41;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 41;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 41;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 41;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 41;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 41;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void StreamingPreferences::displayModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void StreamingPreferences::bitrateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void StreamingPreferences::unlockBitrateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void StreamingPreferences::autoAdjustBitrateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void StreamingPreferences::enableVsyncChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void StreamingPreferences::gameOptimizationsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void StreamingPreferences::playAudioOnHostChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void StreamingPreferences::multiControllerChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void StreamingPreferences::unsupportedFpsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void StreamingPreferences::enableMdnsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void StreamingPreferences::quitAppAfterChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void StreamingPreferences::absoluteMouseModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void StreamingPreferences::absoluteTouchModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void StreamingPreferences::audioConfigChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void StreamingPreferences::videoCodecConfigChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}

// SIGNAL 15
void StreamingPreferences::enableHdrChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 15, nullptr);
}

// SIGNAL 16
void StreamingPreferences::hestiaPreferencesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 16, nullptr);
}

// SIGNAL 17
void StreamingPreferences::enableYUV444Changed()
{
    QMetaObject::activate(this, &staticMetaObject, 17, nullptr);
}

// SIGNAL 18
void StreamingPreferences::videoDecoderSelectionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 18, nullptr);
}

// SIGNAL 19
void StreamingPreferences::uiDisplayModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 19, nullptr);
}

// SIGNAL 20
void StreamingPreferences::windowModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 20, nullptr);
}

// SIGNAL 21
void StreamingPreferences::framePacingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 21, nullptr);
}

// SIGNAL 22
void StreamingPreferences::connectionWarningsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 22, nullptr);
}

// SIGNAL 23
void StreamingPreferences::configurationWarningsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 23, nullptr);
}

// SIGNAL 24
void StreamingPreferences::richPresenceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 24, nullptr);
}

// SIGNAL 25
void StreamingPreferences::gamepadMouseChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 25, nullptr);
}

// SIGNAL 26
void StreamingPreferences::detectNetworkBlockingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 26, nullptr);
}

// SIGNAL 27
void StreamingPreferences::showPerformanceOverlayChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 27, nullptr);
}

// SIGNAL 28
void StreamingPreferences::mouseButtonsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 28, nullptr);
}

// SIGNAL 29
void StreamingPreferences::muteOnFocusLossChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 29, nullptr);
}

// SIGNAL 30
void StreamingPreferences::backgroundGamepadChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 30, nullptr);
}

// SIGNAL 31
void StreamingPreferences::reverseScrollDirectionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 31, nullptr);
}

// SIGNAL 32
void StreamingPreferences::swapFaceButtonsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 32, nullptr);
}

// SIGNAL 33
void StreamingPreferences::captureSysKeysModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 33, nullptr);
}

// SIGNAL 34
void StreamingPreferences::keepAwakeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 34, nullptr);
}

// SIGNAL 35
void StreamingPreferences::languageChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 35, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
