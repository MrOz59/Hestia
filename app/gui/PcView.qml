import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import ComputerModel 1.0

import ComputerManager 1.0
import StreamingPreferences 1.0
import SystemProperties 1.0
import SdlGamepadKeyNavigation 1.0

CenteredGridView {
    property ComputerModel computerModel : createModel()

    id: pcGrid
    focus: true
    activeFocusOnTab: true
    topMargin: 20
    bottomMargin: 5
    cellWidth: 310; cellHeight: 330;
    objectName: qsTr("Computers")

    Component.onCompleted: {
        // Don't show any highlighted item until interacting with them.
        // We do this here instead of onActivated to avoid losing the user's
        // selection when backing out of a different page of the app.
        currentIndex = -1
    }

    // Note: Any initialization done here that is critical for streaming must
    // also be done in CliStartStreamSegue.qml, since this code does not run
    // for command-line initiated streams.
    StackView.onActivated: {
        // Setup signals on CM
        ComputerManager.computerAddCompleted.connect(addComplete)

        // Highlight the first item if a gamepad is connected
        if (currentIndex === -1 && SdlGamepadKeyNavigation.getConnectedGamepads() > 0) {
            currentIndex = 0
        }
    }

    StackView.onDeactivating: {
        ComputerManager.computerAddCompleted.disconnect(addComplete)
    }

    function pairingComplete(error)
    {
        // Close the PIN dialogs
        pairDialog.close()
        otpPairProgressDialog.close()

        // Display a failed dialog if we got an error
        if (error !== undefined) {
            errorDialog.text = error
            errorDialog.helpText = ""
            errorDialog.open()
        }
    }

    function addComplete(success, detectedPortBlocking)
    {
        if (!success) {
            errorDialog.text = qsTr("Unable to connect to the specified PC.")

            if (detectedPortBlocking) {
                errorDialog.text += "\n\n" + qsTr("This PC's Internet connection is blocking Moonlight. Streaming over the Internet may not work while connected to this network.")
            }
            else {
                errorDialog.helpText = qsTr("Click the Help button for possible solutions.")
            }

            errorDialog.open()
        }
    }

    function createModel()
    {
        var model = Qt.createQmlObject('import ComputerModel 1.0; ComputerModel {}', parent, '')
        model.initialize(ComputerManager)
        model.pairingCompleted.connect(pairingComplete)
        model.connectionTestCompleted.connect(testConnectionDialog.connectionTestComplete)
        return model
    }

    Row {
        anchors.centerIn: parent
        spacing: 5
        visible: pcGrid.count === 0

        BusyIndicator {
            id: searchSpinner
            visible: StreamingPreferences.enableMdns
            running: visible
        }

        Label {
            height: searchSpinner.height
            elide: Label.ElideRight
            text: StreamingPreferences.enableMdns ? qsTr("Searching for compatible hosts on your local network...")
                                                  : qsTr("Automatic PC discovery is disabled. Add your PC manually.")
            font.pointSize: 20
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap
        }
    }

    model: computerModel

    delegate: NavigableItemDelegate {
        width: 300; height: 320;
        grid: pcGrid

        property alias pcContextMenu : pcContextMenuLoader.item

        Image {
            id: pcIcon
            anchors.horizontalCenter: parent.horizontalCenter
            source: "qrc:/res/desktop_windows-48px.svg"
            sourceSize {
                width: 200
                height: 200
            }
        }

        Image {
            // TODO: Tooltip
            id: stateIcon
            anchors.horizontalCenter: pcIcon.horizontalCenter
            anchors.verticalCenter: pcIcon.verticalCenter
            anchors.verticalCenterOffset: !model.online ? -18 : -16
            visible: !model.statusUnknown && (!model.online || !model.paired)
            source: !model.online ? "qrc:/res/warning_FILL1_wght300_GRAD200_opsz24.svg" : "qrc:/res/baseline-lock-24px.svg"
            sourceSize {
                width: !model.online ? 75 : 70
                height: !model.online ? 75 : 70
            }
        }

        BusyIndicator {
            id: statusUnknownSpinner
            anchors.horizontalCenter: pcIcon.horizontalCenter
            anchors.verticalCenter: pcIcon.verticalCenter
            anchors.verticalCenterOffset: -15
            width: 75
            height: 75
            visible: model.statusUnknown
            running: visible
        }

        Label {
            id: pcNameText
            text: model.name

            width: parent.width
            anchors.top: pcIcon.bottom
            anchors.bottom: parent.bottom
            font.pointSize: 36
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            elide: Text.ElideRight
        }

        Loader {
            id: pcContextMenuLoader
            asynchronous: true
            sourceComponent: NavigableMenu {
                id: pcContextMenu
                initiator: pcContextMenuLoader.parent
                MenuItem {
                    text: qsTr("PC Status: %1").arg(model.online ? qsTr("Online") : qsTr("Offline"))
                    font.bold: true
                    enabled: false
                }
                NavigableMenuItem {
                    text: qsTr("View All Apps")
                    onTriggered: {
                        var component = Qt.createComponent("AppView.qml")
                        var appView = component.createObject(stackView, {"computerIndex": index, "objectName": model.name, "showHiddenGames": true})
                        stackView.push(appView)
                    }
                    visible: model.online && model.paired
                }
                NavigableMenuItem {
                    text: qsTr("Wake PC")
                    onTriggered: computerModel.wakeComputer(index)
                    visible: !model.online && model.wakeable
                }
                NavigableMenuItem {
                    text: qsTr("Pair using Host Code (OTP)")
                    visible: model.online && !model.paired
                    onTriggered: {
                        otpPairDialog.pcIndex = index
                        otpPinField.text = ""
                        otpPassphraseField.text = ""
                        otpPairDialog.open()
                    }
                }
                NavigableMenuItem {
                    text: qsTr("Test Network")
                    onTriggered: {
                        computerModel.testConnectionForComputer(index)
                        testConnectionDialog.open()
                    }
                }
                NavigableMenuItem {
                    text: qsTr("Hermes Display Status")
                    visible: model.online && model.paired && model.hestiaEnhanced
                    onTriggered: {
                        var status = computerModel.getHestiaDisplayStatus(index)
                        hestiaStatusDialog.text = status.ok ? qsTr("Virtual display: %1\nBackend: %2\nResolution: %3×%4 @ %5 FPS\nKScreen: %6")
                            .arg(status.virtual_display.enabled ? qsTr("Active") : qsTr("Inactive"))
                            .arg(status.virtual_display.backend).arg(status.virtual_display.width).arg(status.virtual_display.height)
                            .arg(status.virtual_display.fps).arg(status.desktop.kscreen_available ? qsTr("Available") : qsTr("Unavailable"))
                            : qsTr("Hermes display status is unavailable for this host.")
                        hestiaStatusDialog.open()
                    }
                }
                NavigableMenuItem {
                    text: qsTr("Recover Physical Monitor")
                    visible: model.online && model.paired && model.hestiaDisplayRecovery
                    onTriggered: { recoverDisplayDialog.pcIndex = index; recoverDisplayDialog.open() }
                }
                NavigableMenuItem {
                    text: qsTr("Hermes Permissions")
                    visible: model.online && model.paired && model.hestiaPermissionSystem
                    onTriggered: {
                        var permissions = computerModel.getHestiaClientPermissions(index)
                        if (!permissions.ok) {
                            hestiaStatusDialog.text = qsTr("Hermes permissions are unavailable for this host.")
                        } else {
                            var p = permissions.permissions
                            hestiaStatusDialog.text = qsTr("Client: %1\n\nStream viewing: %2\nApp listing: %3\nApp launching: %4\nKeyboard input: %5\nMouse input: %6\nGamepad input: %7\nVirtual display: %8\nServer commands: %9\nClipboard sync: %10\nDisplay recovery: %11")
                                    .arg(permissions.client_id)
                                    .arg(p.view_streams ? qsTr("Allowed") : qsTr("Not allowed"))
                                    .arg(p.list_apps ? qsTr("Allowed") : qsTr("Not allowed"))
                                    .arg(p.launch_apps ? qsTr("Allowed") : qsTr("Not allowed"))
                                    .arg(p.keyboard_input ? qsTr("Allowed") : qsTr("Not allowed"))
                                    .arg(p.mouse_input ? qsTr("Allowed") : qsTr("Not allowed"))
                                    .arg(p.gamepad_input ? qsTr("Allowed") : qsTr("Not allowed"))
                                    .arg(p.virtual_display ? qsTr("Allowed") : qsTr("Not allowed"))
                                    .arg(p.server_commands ? qsTr("Allowed") : qsTr("Not allowed"))
                                    .arg(p.clipboard_sync ? qsTr("Allowed") : qsTr("Not allowed"))
                                    .arg(p.display_recovery ? qsTr("Allowed") : qsTr("Not allowed"))
                        }
                        hestiaStatusDialog.open()
                    }
                }
                NavigableMenuItem {
                    text: qsTr("Hermes Diagnostics")
                    visible: model.online && model.paired && model.hestiaEnhanced
                    onTriggered: {
                        var diagnostics = computerModel.getHestiaDiagnostics(index)
                        if (!diagnostics.ok) {
                            hestiaStatusDialog.text = qsTr("Hermes diagnostics are unavailable for this host.")
                            hestiaStatusDialog.open()
                            return
                        }

                        var sections = []

                        var runtime = diagnostics.runtime
                        if (runtime) {
                            var encoder = runtime.encoder
                            if (encoder) {
                                if (encoder.probed === false || !encoder.name) {
                                    sections.push(qsTr("Encoder: not probed yet"))
                                } else {
                                    var codecs = (encoder.codecs && encoder.codecs.length > 0)
                                            ? encoder.codecs.join(", ")
                                            : qsTr("unknown")
                                    sections.push(qsTr("Encoder: %1 (%2)\nCodecs: %3")
                                            .arg(encoder.name)
                                            .arg(encoder.hardware ? qsTr("hardware") : qsTr("software"))
                                            .arg(codecs))
                                }
                            }

                            var sessionsInfo = runtime.sessions
                            if (sessionsInfo) {
                                var sessionLine = qsTr("Active sessions: %1 (%2)")
                                        .arg(sessionsInfo.active)
                                        .arg(sessionsInfo.streaming ? qsTr("streaming") : qsTr("idle"))
                                if (!sessionsInfo.streaming && sessionsInfo.last_termination
                                        && sessionsInfo.last_termination !== "unknown") {
                                    var reasonText
                                    switch (sessionsInfo.last_termination) {
                                    case "client_quit": reasonText = qsTr("client closed the stream"); break
                                    case "client_lost": reasonText = qsTr("client connection lost (network/crash)"); break
                                    case "handshake_failed": reasonText = qsTr("stream handshake failed"); break
                                    case "protocol_error": reasonText = qsTr("protocol error"); break
                                    case "server_stopped": reasonText = qsTr("stopped by the host"); break
                                    case "permission_revoked": reasonText = qsTr("permission revoked"); break
                                    default: reasonText = sessionsInfo.last_termination
                                    }
                                    sessionLine += "\n" + qsTr("Last session ended: %1").arg(reasonText)
                                }
                                sections.push(sessionLine)
                            }

                            var pipeline = runtime.pipeline
                            if (pipeline) {
                                sections.push(qsTr("FPS: %1\nBitrate: %2 kbps\nEncode time: %3 ms\nCapture→encode: %4 ms\nFrames: %5 encoded / %6 dropped")
                                        .arg(Math.round(pipeline.fps))
                                        .arg(Math.round(pipeline.bitrate_kbps))
                                        .arg(pipeline.encode_ms.toFixed(2))
                                        .arg(pipeline.capture_to_encode_ms.toFixed(2))
                                        .arg(pipeline.frames_encoded)
                                        .arg(pipeline.frames_dropped))
                            } else {
                                sections.push(qsTr("Live pipeline metrics: available while streaming"))
                            }
                        }

                        var clipboard = diagnostics.dependencies ? diagnostics.dependencies.clipboard : null
                        if (clipboard) {
                            sections.push(qsTr("Clipboard support: %1\nDiagnostic: %2\n%3")
                                    .arg(clipboard.available ? qsTr("Ready") : qsTr("Unavailable"))
                                    .arg(clipboard.diagnostic)
                                    .arg(clipboard.available ? qsTr("No action is required.") : clipboard.manualInstall))
                        }

                        hestiaStatusDialog.text = sections.length > 0
                                ? sections.join("\n\n")
                                : qsTr("Hermes diagnostics are unavailable for this host.")
                        hestiaStatusDialog.open()
                    }
                }
                NavigableMenuItem {
                    text: qsTr("Paste Host Clipboard")
                    visible: model.online && model.paired && model.hestiaClipboardSync && StreamingPreferences.hestiaClipboardSync
                    onTriggered: { pasteHostClipboardDialog.pcIndex = index; pasteHostClipboardDialog.open() }
                }
                NavigableMenuItem {
                    text: qsTr("Send Local Clipboard")
                    visible: model.online && model.paired && model.hestiaClipboardSync && StreamingPreferences.hestiaClipboardSync
                    onTriggered: { sendLocalClipboardDialog.pcIndex = index; sendLocalClipboardDialog.open() }
                }
                NavigableMenuItem {
                    text: qsTr("Server Commands")
                    visible: model.online && model.paired && model.hestiaServerCommands
                    onTriggered: {
                        var commands = computerModel.getHestiaServerCommands(index)
                        if (commands.length === 0) {
                            hestiaStatusDialog.text = qsTr("No server commands are available for this host or this client lacks permission.")
                            hestiaStatusDialog.open()
                            return
                        }
                        serverCommandsDialog.pcIndex = index
                        serverCommandsDialog.commands = commands
                        serverCommandsDialog.open()
                    }
                }

                NavigableMenuItem {
                    text: qsTr("Rename PC")
                    onTriggered: {
                        renamePcDialog.pcIndex = index
                        renamePcDialog.originalName = model.name
                        renamePcDialog.open()
                    }
                }
                NavigableMenuItem {
                    text: qsTr("Delete PC")
                    onTriggered: {
                        deletePcDialog.pcIndex = index
                        deletePcDialog.pcName = model.name
                        deletePcDialog.open()
                    }
                }
                NavigableMenuItem {
                    text: qsTr("View Details")
                    onTriggered: {
                        showPcDetailsDialog.pcDetails = model.details
                        showPcDetailsDialog.open()
                    }
                }
            }
        }

        onClicked: {
            if (model.online) {
                if (!model.serverSupported) {
                    errorDialog.text = qsTr("The version of GeForce Experience on %1 is not supported by this build of Moonlight. You must update Moonlight to stream from %1.").arg(model.name)
                    errorDialog.helpText = ""
                    errorDialog.open()
                }
                else if (model.paired) {
                    // go to game view
                    var component = Qt.createComponent("AppView.qml")
                    var appView = component.createObject(stackView, {"computerIndex": index, "objectName": model.name})
                    stackView.push(appView)
                }
                else {
                    var pin = computerModel.generatePinString()

                    // Kick off pairing in the background
                    computerModel.pairComputer(index, pin)

                    // Display the pairing dialog
                    pairDialog.pin = pin
                    pairDialog.open()
                }
            } else if (!model.online) {
                // Using open() here because it may be activated by keyboard
                pcContextMenu.open()
            }
        }

        onPressAndHold: {
            // popup() ensures the menu appears under the mouse cursor
            if (pcContextMenu.popup) {
                pcContextMenu.popup()
            }
            else {
                // Qt 5.9 doesn't have popup()
                pcContextMenu.open()
            }
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton;
            onClicked: {
                parent.pressAndHold()
            }
        }

        Keys.onMenuPressed: {
            // We must use open() here so the menu is positioned on
            // the ItemDelegate and not where the mouse cursor is
            pcContextMenu.open()
        }

        Keys.onDeletePressed: {
            deletePcDialog.pcIndex = index
            deletePcDialog.pcName = model.name
            deletePcDialog.open()
        }
    }

    ErrorMessageDialog {
        id: errorDialog

        // Using Setup-Guide here instead of Troubleshooting because it's likely that users
        // will arrive here by forgetting to enable GameStream or not forwarding ports.
        helpUrl: "https://github.com/moonlight-stream/moonlight-docs/wiki/Setup-Guide"
    }

    NavigableMessageDialog {
        id: pairDialog
        closePolicy: Popup.CloseOnEscape

        // don't allow edits to the rest of the window while open
        property string pin : "0000"
        text:qsTr("Please enter %1 on your host PC. This dialog will close when pairing is completed.").arg(pin)+"\n\n"+
             qsTr("If your host PC is running Sunshine, navigate to the Sunshine web UI to enter the PIN.")
        standardButtons: Dialog.Cancel
        onRejected: {
            // FIXME: We should interrupt pairing here
        }
    }

    NavigableDialog {
        id: otpPairDialog
        property int pcIndex: -1
        title: qsTr("Pair using Host Code")
        standardButtons: Dialog.Ok | Dialog.Cancel

        onAccepted: {
            if (otpPinField.text.length === 0) {
                return
            }
            computerModel.pairComputerOtp(pcIndex, otpPinField.text, otpPassphraseField.text)
            otpPairProgressDialog.open()
        }

        ColumnLayout {
            spacing: 8
            Label {
                Layout.maximumWidth: 360
                wrapMode: Text.WordWrap
                text: qsTr("On your host PC, open the Apollo/Hermes web UI and generate a pairing code. Enter the code and passphrase it shows below.")
            }
            Label { text: qsTr("Pairing code (PIN)") }
            TextField {
                id: otpPinField
                Layout.fillWidth: true
                inputMethodHints: Qt.ImhDigitsOnly
                maximumLength: 8
            }
            Label { text: qsTr("Passphrase (optional)") }
            TextField {
                id: otpPassphraseField
                Layout.fillWidth: true
            }
        }
    }

    NavigableMessageDialog {
        id: otpPairProgressDialog
        closePolicy: Popup.CloseOnEscape
        text: qsTr("Pairing with the host. This dialog will close when pairing is completed.")
        standardButtons: Dialog.Cancel
    }

    NavigableMessageDialog {
        id: deletePcDialog
        // don't allow edits to the rest of the window while open
        property int pcIndex : -1
        property string pcName : ""
        text: qsTr("Are you sure you want to remove '%1'?").arg(pcName)
        standardButtons: Dialog.Yes | Dialog.No

        onAccepted: {
            computerModel.deleteComputer(pcIndex)
        }
    }

    NavigableMessageDialog { id: hestiaStatusDialog; standardButtons: Dialog.Ok }
    NavigableMessageDialog {
        id: pasteHostClipboardDialog
        property int pcIndex: -1
        text: qsTr("Replace the local clipboard with text from this host?")
        standardButtons: Dialog.Yes | Dialog.No
        onAccepted: {
            hestiaStatusDialog.text = computerModel.pasteHestiaClipboard(pcIndex) ?
                        qsTr("Host clipboard copied locally.") : qsTr("The host clipboard is unavailable or permission was denied.")
            hestiaStatusDialog.open()
        }
    }
    NavigableMessageDialog {
        id: sendLocalClipboardDialog
        property int pcIndex: -1
        text: qsTr("Send the local clipboard text to this host?")
        standardButtons: Dialog.Yes | Dialog.No
        onAccepted: {
            hestiaStatusDialog.text = computerModel.sendHestiaClipboard(pcIndex) ?
                        qsTr("Local clipboard sent to the host.") : qsTr("The local clipboard could not be sent; check host availability and permission.")
            hestiaStatusDialog.open()
        }
    }
    NavigableMessageDialog {
        id: recoverDisplayDialog
        property int pcIndex: -1
        text: qsTr("Recover the physical monitor? This stops any active stream.")
        standardButtons: Dialog.Yes | Dialog.No
        onAccepted: { hestiaStatusDialog.text = computerModel.recoverHestiaDisplay(pcIndex) ? qsTr("Physical display restored.") : qsTr("Physical display recovery is unavailable."); hestiaStatusDialog.open() }
    }
    NavigableDialog {
        id: serverCommandsDialog
        property int pcIndex: -1
        property var commands: []
        title: qsTr("Server Commands")
        standardButtons: Dialog.Close

        Column {
            spacing: 6
            Repeater {
                model: serverCommandsDialog.commands
                delegate: Button {
                    width: 320
                    text: modelData.name
                    onClicked: {
                        serverCommandConfirmation.pcIndex = serverCommandsDialog.pcIndex
                        serverCommandConfirmation.commandId = modelData.id
                        serverCommandConfirmation.commandName = modelData.name
                        serverCommandConfirmation.open()
                    }
                }
            }
        }
    }
    NavigableMessageDialog {
        id: serverCommandConfirmation
        property int pcIndex: -1
        property int commandId: -1
        property string commandName: ""
        text: qsTr("Run server command '%1'?").arg(commandName)
        standardButtons: Dialog.Yes | Dialog.No
        onAccepted: {
            hestiaStatusDialog.text = computerModel.runHestiaServerCommand(pcIndex, commandId) ?
                        qsTr("Server command started.") : qsTr("The server command could not be started.")
            hestiaStatusDialog.open()
        }
    }

    NavigableMessageDialog {
        id: testConnectionDialog
        closePolicy: Popup.CloseOnEscape
        standardButtons: Dialog.Ok

        onAboutToShow: {
            testConnectionDialog.text = qsTr("Moonlight is testing your network connection to determine if any required ports are blocked.") + "\n\n" + qsTr("This may take a few seconds…")
            showSpinner = true
        }

        function connectionTestComplete(result, blockedPorts)
        {
            if (result === -1) {
                text = qsTr("The network test could not be performed because none of Moonlight's connection testing servers were reachable from this PC. Check your Internet connection or try again later.")
                imageSrc = "qrc:/res/baseline-warning-24px.svg"
            }
            else if (result === 0) {
                text = qsTr("This network does not appear to be blocking Moonlight. If you still have trouble connecting, check your PC's firewall settings.") + "\n\n" + qsTr("If you are trying to stream over the Internet, install the Moonlight Internet Hosting Tool on your gaming PC and run the included Internet Streaming Tester to check your gaming PC's Internet connection.")
                imageSrc = "qrc:/res/baseline-check_circle_outline-24px.svg"
            }
            else {
                text = qsTr("Your PC's current network connection seems to be blocking Moonlight. Streaming over the Internet may not work while connected to this network.") + "\n\n" + qsTr("The following network ports were blocked:") + "\n"
                text += blockedPorts
                imageSrc = "qrc:/res/baseline-error_outline-24px.svg"
            }

            // Stop showing the spinner and show the image instead
            showSpinner = false
        }
    }

    NavigableDialog {
        id: renamePcDialog
        property string label: qsTr("Enter the new name for this PC:")
        property string originalName
        property int pcIndex : -1;

        standardButtons: Dialog.Ok | Dialog.Cancel

        onOpened: {
            // Force keyboard focus on the textbox so keyboard navigation works
            editText.forceActiveFocus()
        }

        onClosed: {
            editText.clear()
        }

        onAccepted: {
            if (editText.text) {
                computerModel.renameComputer(pcIndex, editText.text)
            }
        }

        ColumnLayout {
            Label {
                text: renamePcDialog.label
                font.bold: true
            }

            TextField {
                id: editText
                placeholderText: renamePcDialog.originalName
                Layout.fillWidth: true
                focus: true

                Keys.onReturnPressed: {
                    renamePcDialog.accept()
                }

                Keys.onEnterPressed: {
                    renamePcDialog.accept()
                }
            }
        }
    }

    NavigableMessageDialog {
        id: showPcDetailsDialog
        property string pcDetails : "";
        text: showPcDetailsDialog.pcDetails
        imageSrc: "qrc:/res/baseline-help_outline-24px.svg"
        standardButtons: Dialog.Ok
    }

    ScrollBar.vertical: ScrollBar {}
}
