import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    property string leftText: ""
    property string centerText: ""
    property string rightText: ""
    property bool optionsOpen: false
    property color panelColor: "#0E1418"
    property color borderColor: "#2B3A42"
    property color textColor: "#B9C7C4"
    property color mutedTextColor: "#8DB8B1"
    property color accentTextColor: "#E7BE57"
    property color separatorColor: "#2A3941"
    property real statusFontSize: 12

    signal optionsClicked()

    radius: 8
    border.width: 1
    border.color: borderColor
    color: panelColor

    RowLayout {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 6

        Text {
            Layout.fillWidth: true
            Layout.preferredWidth: 2
            verticalAlignment: Text.AlignVCenter
            text: root.leftText
            color: root.textColor
            font.family: "IBM Plex Mono, Monospace"
            font.pixelSize: root.statusFontSize
            elide: Text.ElideRight
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: root.separatorColor
        }

        Text {
            Layout.fillWidth: true
            Layout.preferredWidth: 2
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            text: root.centerText
            color: root.mutedTextColor
            font.family: "IBM Plex Mono, Monospace"
            font.pixelSize: root.statusFontSize
            elide: Text.ElideRight
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: root.separatorColor
        }

        Text {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight
            text: root.rightText
            color: root.accentTextColor
            font.family: "IBM Plex Mono, Monospace"
            font.pixelSize: root.statusFontSize
            elide: Text.ElideRight
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: root.separatorColor
        }

        Rectangle {
            Layout.preferredWidth: 26
            Layout.fillHeight: true
            radius: 6
            color: root.optionsOpen ? Qt.lighter(root.panelColor, 1.25) : Qt.darker(root.panelColor, 1.08)
            border.width: 1
            border.color: root.borderColor

            Text {
                anchors.centerIn: parent
                text: root.optionsOpen ? "x" : "\u2699"
                color: root.accentTextColor
                font.family: "IBM Plex Mono, Monospace"
                font.pixelSize: root.statusFontSize + 1
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.optionsClicked()
            }
        }
    }
}
