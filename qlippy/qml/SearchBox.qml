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
        radius: 12
        border.width: 1
        border.color: root.borderColor
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.lighter(root.panelColor, 1.06) }
            GradientStop { position: 1.0; color: Qt.darker(root.panelColor, 1.06) }
        }
    }

    leftPadding: horizontalInset
    rightPadding: horizontalInset
    topPadding: verticalInset
    bottomPadding: verticalInset
}
