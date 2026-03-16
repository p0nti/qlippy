import QtQuick 2.15
import QtQuick.Controls 2.15

TextField {
    id: root
    property color textColor: "#F4F1E8"
    property color placeholderColor: "#8FA6A2"
    property color selectionFill: "#2A8C82"
    property color selectionText: "#F4F1E8"
    property color panelColor: "#171D20"
    property color borderColor: "#3D4D4A"
    property color accentColor: "#42C5B8"
    property real fieldFontSize: 16
    property real horizontalInset: 14
    property real verticalInset: 10

    placeholderText: "Search clipboard history"
    placeholderTextColor: placeholderColor
    color: textColor
    selectionColor: selectionFill
    selectedTextColor: selectionText
    font.family: "IBM Plex Sans, Noto Sans, Sans Serif"
    font.pixelSize: fieldFontSize

    background: Rectangle {
        radius: 14
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus ? root.accentColor : Qt.lighter(root.borderColor, 1.12)
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.lighter(root.panelColor, 1.12) }
            GradientStop { position: 0.45; color: Qt.lighter(root.panelColor, 1.06) }
            GradientStop { position: 1.0; color: Qt.darker(root.panelColor, 1.02) }
        }

        Rectangle {
            width: 3
            radius: 2
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.topMargin: 9
            anchors.bottomMargin: 9
            color: Qt.rgba(root.accentColor.r, root.accentColor.g, root.accentColor.b, root.activeFocus ? 0.75 : 0.32)
        }

        Rectangle {
            height: 1
            radius: 1
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            anchors.topMargin: 8
            color: Qt.rgba(1, 1, 1, 0.08)
        }
    }

    leftPadding: horizontalInset
    rightPadding: horizontalInset
    topPadding: verticalInset
    bottomPadding: verticalInset
}
