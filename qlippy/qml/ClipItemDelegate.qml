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
            anchors.verticalCenter: parent.verticalCenter
            Row {
                spacing: 6
                anchors.verticalCenter: parent.verticalCenter
                // PIN indicator (if needed)
                visible: pinned || expandEnabled
                Text {
                    visible: pinned
                    text: "PIN"
                    color: metaTextColor
                    font.family: "IBM Plex Mono, Monospace"
                    font.pixelSize: metaFontSize
                }
                // Chevron indicator for expand/collapse
                Item {
                    visible: expandEnabled
                    width: 18; height: 18
                    anchors.verticalCenter: parent.verticalCenter
                    // Animated chevron
                    Canvas {
                        id: chevronCanvas
                        anchors.fill: parent
                        property real rotation: expanded ? 90 : 0
                        onRotationChanged: chevronAnim.running = true
                        onPaint: {
                            var ctx = getContext('2d');
                            ctx.clearRect(0, 0, width, height);
                            ctx.save();
                            ctx.translate(width/2, height/2);
                            ctx.rotate(rotation * Math.PI / 180);
                            ctx.translate(-width/2, -height/2);
                            ctx.globalAlpha = (current ? 0.95 : (root.hovered ? 0.8 : 0.5));
                            ctx.strokeStyle = current ? currentBorderColor : baseBorderColor;
                            ctx.lineWidth = 2.2;
                            ctx.lineCap = 'round';
                            ctx.beginPath();
                            ctx.moveTo(width*0.35, height*0.28);
                            ctx.lineTo(width*0.65, height*0.5);
                            ctx.lineTo(width*0.35, height*0.72);
                            ctx.stroke();
                            ctx.restore();
                        }
                        Behavior on rotation {
                            NumberAnimation { id: chevronAnim; duration: 150; easing.type: Easing.OutQuad }
                        }
                    }
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        hoverEnabled: true
        onEntered: root.hovered = true
        onExited: root.hovered = false
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

    property bool hovered: false
}
