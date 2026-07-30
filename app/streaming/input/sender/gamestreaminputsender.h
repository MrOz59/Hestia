#pragma once

#include "inputsender.h"

namespace InputSender {

class GameStreamInputSender final : public IInputSender {
public:
    Mode mode() const noexcept override;
    Capabilities capabilities() const noexcept override;

    SendResult sendKeyboard(
            const KeyboardEvent& event) noexcept override;
    SendResult sendText(
            const TextEvent& event) noexcept override;
    SendResult sendMouseButton(
            const MouseButtonEvent& event) noexcept override;
    SendResult sendRelativePointer(
            const RelativePointerEvent& event) noexcept override;
    SendResult sendAbsolutePointer(
            const AbsolutePointerEvent& event) noexcept override;
    SendResult sendScroll(
            const ScrollEvent& event) noexcept override;
    SendResult sendTouch(
            const TouchEvent& event) noexcept override;
    SendResult sendPen(
            const PenEvent& event) noexcept override;
    SendResult sendControllerState(
            const ControllerStateEvent& event) noexcept override;
    SendResult sendControllerArrival(
            const ControllerArrivalEvent& event) noexcept override;
    SendResult sendControllerTouch(
            const ControllerTouchEvent& event) noexcept override;
    SendResult sendControllerMotion(
            const ControllerMotionEvent& event) noexcept override;
    SendResult sendControllerBattery(
            const ControllerBatteryEvent& event) noexcept override;
};

} // namespace InputSender
