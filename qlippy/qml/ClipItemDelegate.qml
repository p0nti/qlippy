import QtQuick 2.15

Rectangle {
    id: root

    property bool current: false
    property bool pinned: false
    property bool expandEnabled: true
    property int row: -1
    property var listView: null
    property var modelRef: null
    property var itemId: 0
    property string typeText: "text"
    property string previewText: ""
    property int timestamp: 0
    property bool expanded: listView && listView.expandedId === itemId
    property real rowHeight: 58
    property real rowPadding: 12
    property real contentGap: 10
    property real previewFontSize: 14
    property real metaFontSize: 12
    property color baseColor: "#171D20"
    property color currentColor: "#1D5A56"
    property color baseBorderColor: "#2F3A3F"
    property color currentBorderColor: "#42C5B8"
    property color bodyTextColor: "#F6F2EA"
    property color metaTextColor: "#F2C14B"
    property color textIndicatorColor: "#5BC18A"
    property color imageIndicatorColor: "#E2A752"

    radius: 12
    clip: true
    color: current ? currentColor : baseColor
    border.width: current ? 2 : 1
    border.color: current ? currentBorderColor : baseBorderColor

    implicitHeight: rowHeight

    Row {
        id: rowContent
        anchors.fill: parent
        anchors.leftMargin: rowPadding
        anchors.rightMargin: rowPadding
        spacing: contentGap

        Rectangle {
            width: 9
            height: 9
            radius: 5
            anchors.verticalCenter: parent.verticalCenter
            color: typeText === "image" ? imageIndicatorColor : textIndicatorColor
        }

        Text {
            id: summaryText
            text: (previewText.length > 0 ? previewText : (typeText === "image" ? "[Image item]" : "[Empty text]"))
                  .replace(/[\r\n\t]+/g, " ")
            color: bodyTextColor
            elide: Text.ElideRight
            wrapMode: Text.NoWrap
            maximumLineCount: 1
            clip: true
            verticalAlignment: Text.AlignVCenter
            width: rowContent.width - meta.width - 34
            anchors.verticalCenter: parent.verticalCenter
            font.family: "IBM Plex Sans, Noto Sans, Sans Serif"
            font.pixelSize: previewFontSize
        }

        Text {
            id: meta
            text: {
                const parts = []
                if (pinned)
                    parts.push("PIN")
                if (expandEnabled)
                    parts.push(expanded ? "[-]" : "[+]")
                return parts.join(" ")
            }
            color: metaTextColor
            anchors.verticalCenter: parent.verticalCenter
            font.family: "IBM Plex Mono, Monospace"
            font.pixelSize: metaFontSize
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onClicked: {
            if (listView) {
                listView.currentIndex = row
                listView.forceActiveFocus()
            }
            if (modelRef)
                modelRef.activate(row)
        }
        onDoubleClicked: function(mouse) {
            // No dedicated double-click behavior.
            mouse.accepted = true
        }
    }
}
