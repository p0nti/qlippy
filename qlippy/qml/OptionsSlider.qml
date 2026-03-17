import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qlippy

Rectangle {
    id: root

    property bool panelOpen: false
    property real panelWidth: 260
    property SettingsModel settingsObject: null
    property var themeData: null
    property var layoutData: null

    signal closeRequested()

    readonly property real sliderInset: layoutData ? layoutData.sliderInset : 14
    readonly property real sectionGap: layoutData ? layoutData.sliderSectionGap : 16
    readonly property real controlHeight: layoutData ? layoutData.sliderControlHeight : 32
    readonly property real tallControlHeight: layoutData ? layoutData.sliderTallControlHeight : 38
    readonly property real buttonSize: layoutData ? layoutData.sliderButtonSize : 24
    readonly property real rowContentInset: Math.max(8, root.sliderInset - 4)
    readonly property real toggleWidth: Math.max(56, root.buttonSize * 2)
    readonly property var themeLabels: ["System Dark", "System Light", "Teal", "Catppuccin"]
    readonly property var themeValues: ["system-dark", "system-light", "teal", "catppuccin"]

    width: panelWidth
    radius: 16
    color: themeData ? Qt.darker(themeData.panel, 1.08) : "#11181B"
    border.width: 1
    border.color: themeData ? themeData.border : "#2F3A3F"
    x: panelOpen ? parent.width - width : parent.width + 12
    opacity: panelOpen ? 1.0 : 0.0
    visible: panelOpen || opacity > 0.0

    Behavior on x {
        NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
    }

    Behavior on opacity {
        NumberAnimation { duration: 140; easing.type: Easing.OutQuad }
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: root.sliderInset
        clip: true

        ColumnLayout {
            width: root.width - (root.sliderInset * 2)
            spacing: root.sectionGap

            RowLayout {
                Layout.fillWidth: true

                Text {
                    Layout.fillWidth: true
                    text: "Options"
                    color: themeData ? themeData.text : "#F4F1E8"
                    font.family: "IBM Plex Sans, Noto Sans, Sans Serif"
                    font.pixelSize: layoutData ? layoutData.titleFontSize : 18
                    font.bold: true
                }

                Rectangle {
                    Layout.preferredWidth: root.buttonSize + 2
                    Layout.preferredHeight: root.buttonSize + 2
                    radius: 8
                    color: themeData ? themeData.panel : "#171D20"
                    border.width: 1
                    border.color: themeData ? themeData.border : "#2F3A3F"

                    Text {
                        anchors.centerIn: parent
                        text: "x"
                        color: themeData ? themeData.text : "#F4F1E8"
                        font.family: "IBM Plex Mono, Monospace"
                        font.pixelSize: layoutData ? layoutData.controlFontSize : 13
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.closeRequested()
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: themeData ? themeData.border : "#2F3A3F"
                opacity: 0.65
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.sectionGap - 6

                Text {
                    text: "Appearance"
                    color: themeData ? themeData.accent : "#42C5B8"
                    font.family: "IBM Plex Mono, Monospace"
                    font.pixelSize: layoutData ? layoutData.sectionFontSize : 12
                }

                Text {
                    text: "Theme"
                    color: themeData ? themeData.muted : "#8FA6A2"
                    font.family: "IBM Plex Sans, Noto Sans, Sans Serif"
                    font.pixelSize: layoutData ? layoutData.labelFontSize : 13
                }

                ComboBox {
                    id: themeSelector
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.controlHeight
                    model: root.themeLabels
                    currentIndex: {
                        if (!settingsObject)
                            return 0
                        for (let i = 0; i < root.themeValues.length; ++i) {
                            if (root.themeValues[i] === settingsObject.theme)
                                return i
                        }
                        return 0
                    }
                    onActivated: {
                        if (settingsObject && currentIndex >= 0 && currentIndex < root.themeValues.length)
                            settingsObject.theme = root.themeValues[currentIndex]
                    }

                    delegate: ItemDelegate {
                        // qmllint disable missing-property
                        required property var modelData
                        required property int index

                        width: themeSelector.width
                        height: root.controlHeight
                        highlighted: themeSelector.highlightedIndex === index
                        padding: 6
                        hoverEnabled: true

                        contentItem: Text {
                            text: modelData
                            color: themeData ? themeData.text : "#F4F1E8"
                            font.family: "IBM Plex Sans, Noto Sans, Sans Serif"
                            font.pixelSize: layoutData ? layoutData.controlFontSize : 13
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }

                        background: Rectangle {
                            radius: 8
                            color: highlighted
                                ? (themeData ? themeData.selectedPanel : "#1D5A56")
                                : (hovered ? (themeData ? Qt.lighter(themeData.panel, 1.08) : "#202A2F") : "transparent")
                            border.width: highlighted ? 1 : (hovered ? 1 : 0)
                            border.color: highlighted
                                ? (themeData ? themeData.accent : "#42C5B8")
                                : (themeData ? themeData.border : "#2F3A3F")
                        }
                        // qmllint enable missing-property
                    }

                    contentItem: Text {
                        leftPadding: 10
                        rightPadding: 24
                        text: themeSelector.displayText
                        color: themeData ? themeData.text : "#F4F1E8"
                        font.family: "IBM Plex Sans, Noto Sans, Sans Serif"
                        font.pixelSize: layoutData ? layoutData.controlFontSize : 13
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }

                    indicator: Canvas {
                        x: themeSelector.width - width - 10
                        y: (themeSelector.height - height) / 2
                        width: 10
                        height: 6
                        contextType: "2d"
                        onPaint: {
                            const ctx = getContext("2d")
                            ctx.reset()
                            ctx.moveTo(0, 0)
                            ctx.lineTo(width, 0)
                            ctx.lineTo(width / 2, height)
                            ctx.closePath()
                            ctx.fillStyle = themeData ? themeData.muted : "#8FA6A2"
                            ctx.fill()
                        }
                    }

                    background: Rectangle {
                        radius: 10
                        color: themeData ? themeData.panel : "#171D20"
                        border.width: 1
                        border.color: themeData ? themeData.border : "#2F3A3F"
                    }

                    popup: Popup {
                        y: themeSelector.height + 4
                        width: themeSelector.width
                        padding: 6
                        implicitHeight: Math.min(contentItem.implicitHeight + 8, root.height * 0.45)

                        contentItem: ListView {
                            clip: true
                            implicitHeight: contentHeight
                            model: themeSelector.delegateModel
                            currentIndex: themeSelector.highlightedIndex
                            spacing: 4
                            ScrollIndicator.vertical: ScrollIndicator {}
                        }

                        background: Rectangle {
                            radius: 10
                            color: themeData ? Qt.darker(themeData.panel, 1.04) : "#171D20"
                            border.width: 1
                            border.color: themeData ? themeData.border : "#2F3A3F"
                        }
                    }
                }

                Text {
                    text: "Layout"
                    color: themeData ? themeData.muted : "#8FA6A2"
                    font.family: "IBM Plex Sans, Noto Sans, Sans Serif"
                    font.pixelSize: layoutData ? layoutData.labelFontSize : 13
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Math.max(4, root.sectionGap - 8)

                    Repeater {
                        model: [
                            { label: "Compact", value: "compact" },
                            { label: "Normal", value: "normal" },
                            { label: "Big", value: "big" }
                        ]

                        delegate: Rectangle {
                            required property var modelData

                            Layout.fillWidth: true
                            Layout.preferredHeight: root.controlHeight
                            radius: 10
                            color: settingsObject && settingsObject.layout === modelData.value
                                ? (themeData ? themeData.selectedPanel : "#1D5A56")
                                : (themeData ? themeData.panel : "#171D20")
                            border.width: 1
                            border.color: settingsObject && settingsObject.layout === modelData.value
                                ? (themeData ? themeData.accent : "#42C5B8")
                                : (themeData ? themeData.border : "#2F3A3F")

                            Text {
                                anchors.centerIn: parent
                                text: modelData.label
                                color: themeData ? themeData.text : "#F4F1E8"
                                font.family: "IBM Plex Sans, Noto Sans, Sans Serif"
                                font.pixelSize: layoutData ? layoutData.controlFontSize : 13
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (settingsObject)
                                        settingsObject.layout = modelData.value
                                }
                            }
                        }
                    }
                }

                Text {
                    text: settingsObject ? "Opacity " + Math.round(settingsObject.opacity * 100) + "%" : "Opacity"
                    color: themeData ? themeData.muted : "#8FA6A2"
                    font.family: "IBM Plex Sans, Noto Sans, Sans Serif"
                    font.pixelSize: layoutData ? layoutData.labelFontSize : 13
                }

                Slider {
                    Layout.fillWidth: true
                    from: 0.6
                    to: 1.0
                    stepSize: 0.05
                    value: settingsObject ? settingsObject.opacity : 1.0
                    onMoved: {
                        if (settingsObject)
                            settingsObject.opacity = value
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.sectionGap - 6

                Text {
                    text: "Behavior"
                    color: themeData ? themeData.accent : "#42C5B8"
                    font.family: "IBM Plex Mono, Monospace"
                    font.pixelSize: layoutData ? layoutData.sectionFontSize : 12
                }

                Repeater {
                    model: [
                        { label: "Expand mode", key: "expandMode" },
                        { label: "Adjust images when in expand mode", key: "compactImageExpand" },
                        { label: "Dedupe", key: "dedupe" },
                        { label: "Save images", key: "saveImages" },
                        { label: "Allow deletion items", key: "allowDeletionItems" }
                    ]

                    delegate: Rectangle {
                        required property var modelData

                        Layout.fillWidth: true
                        Layout.preferredHeight: root.tallControlHeight
                        radius: 10
                        color: themeData ? themeData.panel : "#171D20"
                        border.width: 1
                        border.color: themeData ? themeData.border : "#2F3A3F"

                        RowLayout {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.leftMargin: root.rowContentInset
                            anchors.rightMargin: root.rowContentInset
                            anchors.verticalCenter: parent.verticalCenter

                            Text {
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignVCenter
                                text: modelData.label
                                color: themeData ? themeData.text : "#F4F1E8"
                                font.family: "IBM Plex Sans, Noto Sans, Sans Serif"
                                font.pixelSize: layoutData ? layoutData.controlFontSize : 13
                                elide: Text.ElideRight
                            }

                            Rectangle {
                                Layout.alignment: Qt.AlignVCenter
                                Layout.preferredWidth: root.toggleWidth
                                Layout.preferredHeight: root.buttonSize
                                radius: 12
                                color: settingsObject && settingsObject[modelData.key]
                                    ? (themeData ? themeData.selectedPanel : "#1D5A56")
                                    : (themeData ? Qt.darker(themeData.panel, 1.1) : "#12181B")
                                border.width: 1
                                border.color: settingsObject && settingsObject[modelData.key]
                                    ? (themeData ? themeData.accent : "#42C5B8")
                                    : (themeData ? themeData.border : "#2F3A3F")

                                Text {
                                    anchors.centerIn: parent
                                    text: settingsObject && settingsObject[modelData.key] ? "ON" : "OFF"
                                    color: themeData ? themeData.text : "#F4F1E8"
                                    font.family: "IBM Plex Mono, Monospace"
                                    font.pixelSize: layoutData ? layoutData.controlFontSize : 12
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        if (settingsObject)
                                            settingsObject[modelData.key] = !settingsObject[modelData.key]
                                    }
                                }
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.sectionGap - 6

                Text {
                    text: "History"
                    color: themeData ? themeData.accent : "#42C5B8"
                    font.family: "IBM Plex Mono, Monospace"
                    font.pixelSize: layoutData ? layoutData.sectionFontSize : 12
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.tallControlHeight + 4
                    radius: 10
                    color: themeData ? themeData.panel : "#171D20"
                    border.width: 1
                    border.color: themeData ? themeData.border : "#2F3A3F"

                    RowLayout {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: root.rowContentInset
                        anchors.rightMargin: root.rowContentInset
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter
                            text: "Max history"
                            color: themeData ? themeData.text : "#F4F1E8"
                            font.family: "IBM Plex Sans, Noto Sans, Sans Serif"
                            font.pixelSize: layoutData ? layoutData.controlFontSize : 13
                        }

                        Rectangle {
                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredWidth: root.buttonSize
                            Layout.preferredHeight: root.buttonSize
                            radius: 8
                            color: themeData ? Qt.darker(themeData.panel, 1.1) : "#12181B"
                            border.width: 1
                            border.color: themeData ? themeData.border : "#2F3A3F"

                            Text {
                                anchors.centerIn: parent
                                text: "-"
                                color: themeData ? themeData.text : "#F4F1E8"
                                font.family: "IBM Plex Mono, Monospace"
                                font.pixelSize: layoutData ? layoutData.controlFontSize : 14
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (settingsObject)
                                        settingsObject.maxHistory = Math.max(1, settingsObject.maxHistory - 25)
                                }
                            }
                        }

                        Text {
                            Layout.alignment: Qt.AlignVCenter
                            text: settingsObject ? String(settingsObject.maxHistory) : "500"
                            color: themeData ? themeData.accent : "#42C5B8"
                            font.family: "IBM Plex Mono, Monospace"
                            font.pixelSize: layoutData ? layoutData.controlFontSize : 13
                        }

                        Rectangle {
                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredWidth: root.buttonSize
                            Layout.preferredHeight: root.buttonSize
                            radius: 8
                            color: themeData ? Qt.darker(themeData.panel, 1.1) : "#12181B"
                            border.width: 1
                            border.color: themeData ? themeData.border : "#2F3A3F"

                            Text {
                                anchors.centerIn: parent
                                text: "+"
                                color: themeData ? themeData.text : "#F4F1E8"
                                font.family: "IBM Plex Mono, Monospace"
                                font.pixelSize: layoutData ? layoutData.controlFontSize : 14
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (settingsObject)
                                        settingsObject.maxHistory = settingsObject.maxHistory + 25
                                }
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.sectionGap - 6

                Text {
                    text: "Controls"
                    color: themeData ? themeData.accent : "#42C5B8"
                    font.family: "IBM Plex Mono, Monospace"
                    font.pixelSize: layoutData ? layoutData.sectionFontSize : 12
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: controlsColumn.implicitHeight + (root.rowContentInset * 2)
                    radius: 10
                    color: themeData ? themeData.panel : "#171D20"
                    border.width: 1
                    border.color: themeData ? themeData.border : "#2F3A3F"

                    Column {
                        id: controlsColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: root.rowContentInset
                        spacing: 6

                        Text {
                            text: "Keyboard"
                            color: themeData ? themeData.muted : "#8FA6A2"
                            font.family: "IBM Plex Mono, Monospace"
                            font.pixelSize: layoutData ? layoutData.labelFontSize : 13
                        }

                        Text {
                            width: parent.width
                            wrapMode: Text.WordWrap
                            text: "Enter\tCopy selected entry\nDel\tDelete selected entry\nP\tPin selected entry\nSpace\tExpand or collapse preview\nEsc\tHide popup"
                            color: themeData ? themeData.text : "#F4F1E8"
                            font.family: "IBM Plex Mono, Monospace"
                            font.pixelSize: layoutData ? layoutData.controlFontSize : 13
                        }

                        Text {
                            text: "Mouse"
                            color: themeData ? themeData.muted : "#8FA6A2"
                            font.family: "IBM Plex Mono, Monospace"
                            font.pixelSize: layoutData ? layoutData.labelFontSize : 13
                        }

                        Text {
                            width: parent.width
                            wrapMode: Text.WordWrap
                            text: "Hover\tSelect entry under cursor\nClick\tCopy selected entry"
                            color: themeData ? themeData.text : "#F4F1E8"
                            font.family: "IBM Plex Mono, Monospace"
                            font.pixelSize: layoutData ? layoutData.controlFontSize : 13
                        }
                    }
                }
            }
        }
    }
}