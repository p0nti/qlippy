import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root

    property bool current: false
    property bool pinned: false
    property string typeText: "text"
    property string previewText: ""
    property int timestamp: 0

    radius: 12
    color: current ? "#1D5A56" : "#171D20"
    border.width: current ? 2 : 1
    border.color: current ? "#42C5B8" : "#2F3A3F"

    implicitHeight: 58

    Row {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 10

        Rectangle {
            width: 9
            height: 9
            radius: 5
            anchors.verticalCenter: parent.verticalCenter
            color: typeText === "image" ? "#E2A752" : "#5BC18A"
        }

        Text {
            text: previewText.length > 0 ? previewText : (typeText === "image" ? "[Image item]" : "[Empty text]")
            color: "#F6F2EA"
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            width: parent.width - meta.width - 34
            anchors.verticalCenter: parent.verticalCenter
            font.family: "IBM Plex Sans, Noto Sans, Sans Serif"
            font.pixelSize: 14
        }

        Text {
            id: meta
            text: pinned ? "PIN" : ""
            color: "#F2C14B"
            anchors.verticalCenter: parent.verticalCenter
            font.family: "IBM Plex Mono, Monospace"
            font.pixelSize: 12
        }
    }
}
