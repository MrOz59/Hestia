#include "gamestreaminputsender.h"

#include <Limelight.h>

namespace {

InputSender::SendResult mapResult(int errorCode) noexcept
{
    if (errorCode == 0) {
        return {InputSender::SendStatus::Accepted, 0};
    }
    if (errorCode == LI_ERR_UNSUPPORTED) {
        return {InputSender::SendStatus::Unsupported, errorCode};
    }
    return {InputSender::SendStatus::Failed, errorCode};
}

char mapKeyAction(InputSender::KeyAction action) noexcept
{
    return action == InputSender::KeyAction::Down ?
               KEY_ACTION_DOWN :
               KEY_ACTION_UP;
}

char mapModifiers(uint8_t modifiers) noexcept
{
    char mapped = 0;
    if (modifiers & InputSender::KeyboardModifierShift) {
        mapped |= MODIFIER_SHIFT;
    }
    if (modifiers & InputSender::KeyboardModifierControl) {
        mapped |= MODIFIER_CTRL;
    }
    if (modifiers & InputSender::KeyboardModifierAlt) {
        mapped |= MODIFIER_ALT;
    }
    if (modifiers & InputSender::KeyboardModifierMeta) {
        mapped |= MODIFIER_META;
    }
    return mapped;
}

char mapButtonAction(InputSender::ButtonAction action) noexcept
{
    return action == InputSender::ButtonAction::Press ?
               BUTTON_ACTION_PRESS :
               BUTTON_ACTION_RELEASE;
}

int mapMouseButton(InputSender::MouseButton button) noexcept
{
    switch (button) {
    case InputSender::MouseButton::Left:
        return BUTTON_LEFT;
    case InputSender::MouseButton::Middle:
        return BUTTON_MIDDLE;
    case InputSender::MouseButton::Right:
        return BUTTON_RIGHT;
    case InputSender::MouseButton::Extra1:
        return BUTTON_X1;
    case InputSender::MouseButton::Extra2:
        return BUTTON_X2;
    }
    return BUTTON_LEFT;
}

uint8_t mapContactType(
        InputSender::ContactEventType type) noexcept
{
    switch (type) {
    case InputSender::ContactEventType::Hover:
        return LI_TOUCH_EVENT_HOVER;
    case InputSender::ContactEventType::Down:
        return LI_TOUCH_EVENT_DOWN;
    case InputSender::ContactEventType::Up:
        return LI_TOUCH_EVENT_UP;
    case InputSender::ContactEventType::Move:
        return LI_TOUCH_EVENT_MOVE;
    case InputSender::ContactEventType::Cancel:
        return LI_TOUCH_EVENT_CANCEL;
    case InputSender::ContactEventType::ButtonOnly:
        return LI_TOUCH_EVENT_BUTTON_ONLY;
    case InputSender::ContactEventType::HoverLeave:
        return LI_TOUCH_EVENT_HOVER_LEAVE;
    case InputSender::ContactEventType::CancelAll:
        return LI_TOUCH_EVENT_CANCEL_ALL;
    }
    return LI_TOUCH_EVENT_CANCEL;
}

uint8_t mapPenTool(InputSender::PenTool tool) noexcept
{
    switch (tool) {
    case InputSender::PenTool::Unknown:
        return LI_TOOL_TYPE_UNKNOWN;
    case InputSender::PenTool::Pen:
        return LI_TOOL_TYPE_PEN;
    case InputSender::PenTool::Eraser:
        return LI_TOOL_TYPE_ERASER;
    }
    return LI_TOOL_TYPE_UNKNOWN;
}

uint8_t mapPenButtons(uint8_t buttons) noexcept
{
    uint8_t mapped = 0;
    if (buttons & InputSender::PenButtonPrimary) {
        mapped |= LI_PEN_BUTTON_PRIMARY;
    }
    if (buttons & InputSender::PenButtonSecondary) {
        mapped |= LI_PEN_BUTTON_SECONDARY;
    }
    if (buttons & InputSender::PenButtonTertiary) {
        mapped |= LI_PEN_BUTTON_TERTIARY;
    }
    return mapped;
}

int mapControllerButtons(uint32_t buttons) noexcept
{
    int mapped = 0;
    if (buttons & InputSender::ControllerButtonA) {
        mapped |= A_FLAG;
    }
    if (buttons & InputSender::ControllerButtonB) {
        mapped |= B_FLAG;
    }
    if (buttons & InputSender::ControllerButtonX) {
        mapped |= X_FLAG;
    }
    if (buttons & InputSender::ControllerButtonY) {
        mapped |= Y_FLAG;
    }
    if (buttons & InputSender::ControllerButtonBack) {
        mapped |= BACK_FLAG;
    }
    if (buttons & InputSender::ControllerButtonGuide) {
        mapped |= SPECIAL_FLAG;
    }
    if (buttons & InputSender::ControllerButtonStart) {
        mapped |= PLAY_FLAG;
    }
    if (buttons & InputSender::ControllerButtonLeftStick) {
        mapped |= LS_CLK_FLAG;
    }
    if (buttons & InputSender::ControllerButtonRightStick) {
        mapped |= RS_CLK_FLAG;
    }
    if (buttons & InputSender::ControllerButtonLeftShoulder) {
        mapped |= LB_FLAG;
    }
    if (buttons & InputSender::ControllerButtonRightShoulder) {
        mapped |= RB_FLAG;
    }
    if (buttons & InputSender::ControllerButtonDpadUp) {
        mapped |= UP_FLAG;
    }
    if (buttons & InputSender::ControllerButtonDpadDown) {
        mapped |= DOWN_FLAG;
    }
    if (buttons & InputSender::ControllerButtonDpadLeft) {
        mapped |= LEFT_FLAG;
    }
    if (buttons & InputSender::ControllerButtonDpadRight) {
        mapped |= RIGHT_FLAG;
    }
    if (buttons & InputSender::ControllerButtonMisc) {
        mapped |= MISC_FLAG;
    }
    if (buttons & InputSender::ControllerButtonPaddle1) {
        mapped |= PADDLE1_FLAG;
    }
    if (buttons & InputSender::ControllerButtonPaddle2) {
        mapped |= PADDLE2_FLAG;
    }
    if (buttons & InputSender::ControllerButtonPaddle3) {
        mapped |= PADDLE3_FLAG;
    }
    if (buttons & InputSender::ControllerButtonPaddle4) {
        mapped |= PADDLE4_FLAG;
    }
    if (buttons & InputSender::ControllerButtonTouchpad) {
        mapped |= TOUCHPAD_FLAG;
    }
    return mapped;
}

uint8_t mapControllerKind(
        InputSender::ControllerKind kind) noexcept
{
    switch (kind) {
    case InputSender::ControllerKind::Unknown:
        return LI_CTYPE_UNKNOWN;
    case InputSender::ControllerKind::Xbox:
        return LI_CTYPE_XBOX;
    case InputSender::ControllerKind::PlayStation:
        return LI_CTYPE_PS;
    case InputSender::ControllerKind::Nintendo:
        return LI_CTYPE_NINTENDO;
    }
    return LI_CTYPE_UNKNOWN;
}

uint16_t mapControllerCapabilities(uint16_t capabilities) noexcept
{
    uint16_t mapped = 0;
    if (capabilities & InputSender::ControllerCapabilityAnalogTriggers) {
        mapped |= LI_CCAP_ANALOG_TRIGGERS;
    }
    if (capabilities & InputSender::ControllerCapabilityRumble) {
        mapped |= LI_CCAP_RUMBLE;
    }
    if (capabilities & InputSender::ControllerCapabilityTriggerRumble) {
        mapped |= LI_CCAP_TRIGGER_RUMBLE;
    }
    if (capabilities & InputSender::ControllerCapabilityTouchpad) {
        mapped |= LI_CCAP_TOUCHPAD;
    }
    if (capabilities & InputSender::ControllerCapabilityAccelerometer) {
        mapped |= LI_CCAP_ACCEL;
    }
    if (capabilities & InputSender::ControllerCapabilityGyroscope) {
        mapped |= LI_CCAP_GYRO;
    }
    if (capabilities & InputSender::ControllerCapabilityBattery) {
        mapped |= LI_CCAP_BATTERY_STATE;
    }
    if (capabilities & InputSender::ControllerCapabilityRgbLed) {
        mapped |= LI_CCAP_RGB_LED;
    }
    if (capabilities & InputSender::ControllerCapabilityDualTouchpad) {
        mapped |= LI_CCAP_DUAL_TOUCHPAD;
    }
    return mapped;
}

uint8_t mapBatteryState(InputSender::BatteryState state) noexcept
{
    switch (state) {
    case InputSender::BatteryState::Unknown:
        return LI_BATTERY_STATE_UNKNOWN;
    case InputSender::BatteryState::NotPresent:
        return LI_BATTERY_STATE_NOT_PRESENT;
    case InputSender::BatteryState::Discharging:
        return LI_BATTERY_STATE_DISCHARGING;
    case InputSender::BatteryState::Charging:
        return LI_BATTERY_STATE_CHARGING;
    case InputSender::BatteryState::NotCharging:
        return LI_BATTERY_STATE_NOT_CHARGING;
    case InputSender::BatteryState::Full:
        return LI_BATTERY_STATE_FULL;
    }
    return LI_BATTERY_STATE_UNKNOWN;
}

} // namespace

namespace InputSender {

Mode GameStreamInputSender::mode() const noexcept
{
    return Mode::GameStream;
}

Capabilities GameStreamInputSender::capabilities() const noexcept
{
    const int hostFeatures = LiGetHostFeatureFlags();
    return {
        (hostFeatures & LI_FF_PEN_TOUCH_EVENTS) != 0,
        (hostFeatures & LI_FF_CONTROLLER_TOUCH_EVENTS) != 0,
    };
}

SendResult GameStreamInputSender::sendKeyboard(
        const KeyboardEvent& event) noexcept
{
    return mapResult(LiSendKeyboardEvent2(
            static_cast<short>(0x8000U | event.virtualKey),
            mapKeyAction(event.action),
            mapModifiers(event.modifiers),
            event.nonNormalized ? SS_KBE_FLAG_NON_NORMALIZED : 0));
}

SendResult GameStreamInputSender::sendText(
        const TextEvent& event) noexcept
{
    return mapResult(LiSendUtf8TextEvent(
            event.utf8.data(),
            static_cast<unsigned int>(event.utf8.size())));
}

SendResult GameStreamInputSender::sendMouseButton(
        const MouseButtonEvent& event) noexcept
{
    return mapResult(LiSendMouseButtonEvent(
            mapButtonAction(event.action),
            mapMouseButton(event.button)));
}

SendResult GameStreamInputSender::sendRelativePointer(
        const RelativePointerEvent& event) noexcept
{
    return mapResult(LiSendMouseMoveEvent(
            static_cast<short>(event.deltaX),
            static_cast<short>(event.deltaY)));
}

SendResult GameStreamInputSender::sendAbsolutePointer(
        const AbsolutePointerEvent& event) noexcept
{
    return mapResult(LiSendMousePositionEvent(
            static_cast<short>(event.x),
            static_cast<short>(event.y),
            static_cast<short>(event.referenceWidth),
            static_cast<short>(event.referenceHeight)));
}

SendResult GameStreamInputSender::sendScroll(
        const ScrollEvent& event) noexcept
{
    if (event.unit == ScrollUnit::WheelDelta) {
        return mapResult(event.axis == ScrollAxis::Vertical ?
                             LiSendHighResScrollEvent(event.delta) :
                             LiSendHighResHScrollEvent(event.delta));
    }

    return mapResult(event.axis == ScrollAxis::Vertical ?
                         LiSendScrollEvent(
                             static_cast<signed char>(event.delta)) :
                         LiSendHScrollEvent(
                             static_cast<signed char>(event.delta)));
}

SendResult GameStreamInputSender::sendTouch(
        const TouchEvent& event) noexcept
{
    return mapResult(LiSendTouchEvent(
            mapContactType(event.type),
            event.pointerId,
            event.x,
            event.y,
            event.pressureOrDistance,
            event.contactAreaMajor,
            event.contactAreaMinor,
            event.rotation));
}

SendResult GameStreamInputSender::sendPen(
        const PenEvent& event) noexcept
{
    return mapResult(LiSendPenEvent(
            mapContactType(event.type),
            mapPenTool(event.tool),
            mapPenButtons(event.buttons),
            event.x,
            event.y,
            event.pressureOrDistance,
            event.contactAreaMajor,
            event.contactAreaMinor,
            event.rotation,
            event.tilt == UnknownTilt ?
                LI_TILT_UNKNOWN :
                static_cast<uint8_t>(event.tilt)));
}

SendResult GameStreamInputSender::sendControllerState(
        const ControllerStateEvent& event) noexcept
{
    return mapResult(LiSendMultiControllerEvent(
            static_cast<short>(event.controllerId),
            static_cast<short>(event.activeControllerMask),
            mapControllerButtons(event.buttons),
            event.leftTrigger,
            event.rightTrigger,
            event.leftStickX,
            event.leftStickY,
            event.rightStickX,
            event.rightStickY));
}

SendResult GameStreamInputSender::sendControllerArrival(
        const ControllerArrivalEvent& event) noexcept
{
    return mapResult(LiSendControllerArrivalEvent(
            static_cast<uint8_t>(event.controllerId),
            event.activeControllerMask,
            mapControllerKind(event.kind),
            static_cast<uint32_t>(
                mapControllerButtons(event.supportedButtons)),
            mapControllerCapabilities(event.capabilities)));
}

SendResult GameStreamInputSender::sendControllerTouch(
        const ControllerTouchEvent& event) noexcept
{
    return mapResult(LiSendControllerTouchEvent2(
            static_cast<uint8_t>(event.controllerId),
            mapContactType(event.type),
            event.touchpadIndex,
            event.pointerId,
            event.x,
            event.y,
            event.pressure));
}

SendResult GameStreamInputSender::sendControllerMotion(
        const ControllerMotionEvent& event) noexcept
{
    return mapResult(LiSendControllerMotionEvent(
            static_cast<uint8_t>(event.controllerId),
            event.type == MotionType::Accelerometer ?
                LI_MOTION_TYPE_ACCEL :
                LI_MOTION_TYPE_GYRO,
            event.x,
            event.y,
            event.z));
}

SendResult GameStreamInputSender::sendControllerBattery(
        const ControllerBatteryEvent& event) noexcept
{
    return mapResult(LiSendControllerBatteryEvent(
            static_cast<uint8_t>(event.controllerId),
            mapBatteryState(event.state),
            event.percentage));
}

} // namespace InputSender
