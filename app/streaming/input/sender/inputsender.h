#pragma once

#include <cstdint>
#include <string>

namespace InputSender {

enum class Mode : uint8_t {
    GameStream,
};

enum class SendStatus : uint8_t {
    Accepted,
    Unsupported,
    Failed,
};

struct SendResult {
    SendStatus status = SendStatus::Failed;
    int errorCode = 0;

    explicit operator bool() const noexcept
    {
        return status == SendStatus::Accepted;
    }
};

struct Capabilities {
    bool nativePenTouch = false;
    bool controllerTouch = false;
};

// Metadata is produced at the SDL capture boundary. Sequence values are
// session-local and timestamps use the client's monotonic clock.
struct EventMetadata {
    uint64_t sequence = 0;
    uint64_t timestampUs = 0;
    uint32_t deviceId = 0;
    bool replaceable = false;
};

enum class KeyAction : uint8_t {
    Down,
    Up,
};

enum KeyboardModifier : uint8_t {
    KeyboardModifierNone = 0,
    KeyboardModifierShift = 1U << 0,
    KeyboardModifierControl = 1U << 1,
    KeyboardModifierAlt = 1U << 2,
    KeyboardModifierMeta = 1U << 3,
};

struct KeyboardEvent {
    EventMetadata metadata;
    // The current SDL translator produces Win32 virtual-key codes. Keeping the
    // code space explicit allows a native sender to add USB HID without
    // leaking a GameStream packet structure into the capture layer.
    uint16_t virtualKey = 0;
    KeyAction action = KeyAction::Up;
    uint8_t modifiers = KeyboardModifierNone;
    bool nonNormalized = false;
};

struct TextEvent {
    EventMetadata metadata;
    std::string utf8;
};

enum class MouseButton : uint8_t {
    Left,
    Middle,
    Right,
    Extra1,
    Extra2,
};

enum class ButtonAction : uint8_t {
    Press,
    Release,
};

struct MouseButtonEvent {
    EventMetadata metadata;
    MouseButton button = MouseButton::Left;
    ButtonAction action = ButtonAction::Release;
};

struct RelativePointerEvent {
    EventMetadata metadata;
    int32_t deltaX = 0;
    int32_t deltaY = 0;
};

struct AbsolutePointerEvent {
    EventMetadata metadata;
    int32_t x = 0;
    int32_t y = 0;
    int32_t referenceWidth = 0;
    int32_t referenceHeight = 0;
};

enum class ScrollAxis : uint8_t {
    Vertical,
    Horizontal,
};

enum class ScrollUnit : uint8_t {
    Clicks,
    WheelDelta,
};

struct ScrollEvent {
    EventMetadata metadata;
    ScrollAxis axis = ScrollAxis::Vertical;
    ScrollUnit unit = ScrollUnit::Clicks;
    int16_t delta = 0;
};

enum class ContactEventType : uint8_t {
    Hover,
    Down,
    Up,
    Move,
    Cancel,
    ButtonOnly,
    HoverLeave,
    CancelAll,
};

constexpr uint16_t UnknownRotation = UINT16_MAX;
constexpr int16_t UnknownTilt = -1;

struct TouchEvent {
    EventMetadata metadata;
    ContactEventType type = ContactEventType::Move;
    uint32_t pointerId = 0;
    float x = 0.0f;
    float y = 0.0f;
    float pressureOrDistance = 0.0f;
    float contactAreaMajor = 0.0f;
    float contactAreaMinor = 0.0f;
    uint16_t rotation = UnknownRotation;
};

enum class PenTool : uint8_t {
    Unknown,
    Pen,
    Eraser,
};

enum PenButton : uint8_t {
    PenButtonNone = 0,
    PenButtonPrimary = 1U << 0,
    PenButtonSecondary = 1U << 1,
    PenButtonTertiary = 1U << 2,
};

struct PenEvent {
    EventMetadata metadata;
    ContactEventType type = ContactEventType::Move;
    PenTool tool = PenTool::Unknown;
    uint8_t buttons = PenButtonNone;
    float x = 0.0f;
    float y = 0.0f;
    float pressureOrDistance = 0.0f;
    float contactAreaMajor = 0.0f;
    float contactAreaMinor = 0.0f;
    uint16_t rotation = UnknownRotation;
    int16_t tilt = UnknownTilt;
};

// Semantic button mask used inside Hestia. The GameStream adapter is
// responsible for translating it to the legacy wire layout.
enum ControllerButton : uint32_t {
    ControllerButtonNone = 0,
    ControllerButtonA = 1U << 0,
    ControllerButtonB = 1U << 1,
    ControllerButtonX = 1U << 2,
    ControllerButtonY = 1U << 3,
    ControllerButtonBack = 1U << 4,
    ControllerButtonGuide = 1U << 5,
    ControllerButtonStart = 1U << 6,
    ControllerButtonLeftStick = 1U << 7,
    ControllerButtonRightStick = 1U << 8,
    ControllerButtonLeftShoulder = 1U << 9,
    ControllerButtonRightShoulder = 1U << 10,
    ControllerButtonDpadUp = 1U << 11,
    ControllerButtonDpadDown = 1U << 12,
    ControllerButtonDpadLeft = 1U << 13,
    ControllerButtonDpadRight = 1U << 14,
    ControllerButtonMisc = 1U << 15,
    ControllerButtonPaddle1 = 1U << 16,
    ControllerButtonPaddle2 = 1U << 17,
    ControllerButtonPaddle3 = 1U << 18,
    ControllerButtonPaddle4 = 1U << 19,
    ControllerButtonTouchpad = 1U << 20,
};

struct ControllerStateEvent {
    EventMetadata metadata;
    uint16_t controllerId = 0;
    uint16_t activeControllerMask = 0;
    uint32_t buttons = ControllerButtonNone;
    uint8_t leftTrigger = 0;
    uint8_t rightTrigger = 0;
    int16_t leftStickX = 0;
    int16_t leftStickY = 0;
    int16_t rightStickX = 0;
    int16_t rightStickY = 0;
};

enum class ControllerKind : uint8_t {
    Unknown,
    Xbox,
    PlayStation,
    Nintendo,
};

enum ControllerCapability : uint16_t {
    ControllerCapabilityNone = 0,
    ControllerCapabilityAnalogTriggers = 1U << 0,
    ControllerCapabilityRumble = 1U << 1,
    ControllerCapabilityTriggerRumble = 1U << 2,
    ControllerCapabilityTouchpad = 1U << 3,
    ControllerCapabilityAccelerometer = 1U << 4,
    ControllerCapabilityGyroscope = 1U << 5,
    ControllerCapabilityBattery = 1U << 6,
    ControllerCapabilityRgbLed = 1U << 7,
    ControllerCapabilityDualTouchpad = 1U << 8,
};

struct ControllerArrivalEvent {
    EventMetadata metadata;
    uint16_t controllerId = 0;
    uint16_t activeControllerMask = 0;
    ControllerKind kind = ControllerKind::Unknown;
    uint32_t supportedButtons = ControllerButtonNone;
    uint16_t capabilities = ControllerCapabilityNone;
};

struct ControllerTouchEvent {
    EventMetadata metadata;
    uint16_t controllerId = 0;
    ContactEventType type = ContactEventType::Move;
    uint8_t touchpadIndex = 0;
    uint32_t pointerId = 0;
    float x = 0.0f;
    float y = 0.0f;
    float pressure = 0.0f;
};

enum class MotionType : uint8_t {
    Accelerometer,
    Gyroscope,
};

struct ControllerMotionEvent {
    EventMetadata metadata;
    uint16_t controllerId = 0;
    MotionType type = MotionType::Accelerometer;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

enum class BatteryState : uint8_t {
    Unknown,
    NotPresent,
    Discharging,
    Charging,
    NotCharging,
    Full,
};

constexpr uint8_t UnknownBatteryPercentage = UINT8_MAX;

struct ControllerBatteryEvent {
    EventMetadata metadata;
    uint16_t controllerId = 0;
    BatteryState state = BatteryState::Unknown;
    uint8_t percentage = UnknownBatteryPercentage;
};

class IInputSender {
public:
    virtual ~IInputSender() = default;

    // Implementations must accept calls from SDL timer threads as well as the
    // main event thread. Events from all callers share one session sequence.
    virtual Mode mode() const noexcept = 0;
    virtual Capabilities capabilities() const noexcept = 0;

    virtual SendResult sendKeyboard(
            const KeyboardEvent& event) noexcept = 0;
    virtual SendResult sendText(
            const TextEvent& event) noexcept = 0;
    virtual SendResult sendMouseButton(
            const MouseButtonEvent& event) noexcept = 0;
    virtual SendResult sendRelativePointer(
            const RelativePointerEvent& event) noexcept = 0;
    virtual SendResult sendAbsolutePointer(
            const AbsolutePointerEvent& event) noexcept = 0;
    virtual SendResult sendScroll(
            const ScrollEvent& event) noexcept = 0;
    virtual SendResult sendTouch(
            const TouchEvent& event) noexcept = 0;
    virtual SendResult sendPen(
            const PenEvent& event) noexcept = 0;
    virtual SendResult sendControllerState(
            const ControllerStateEvent& event) noexcept = 0;
    virtual SendResult sendControllerArrival(
            const ControllerArrivalEvent& event) noexcept = 0;
    virtual SendResult sendControllerTouch(
            const ControllerTouchEvent& event) noexcept = 0;
    virtual SendResult sendControllerMotion(
            const ControllerMotionEvent& event) noexcept = 0;
    virtual SendResult sendControllerBattery(
            const ControllerBatteryEvent& event) noexcept = 0;
};

} // namespace InputSender
