import QtQuick
import Qlippy

Rectangle {
    id: root

    property bool current: false
    property bool pinned: false
    property bool expandEnabled: true
    property int row: -1
    property var listView: null
    property ClipboardModel modelRef: null
    property int itemId: 0
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
    property bool showDeleteIcon: false
    property color deleteIconColor: "#E2A752"

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
        z: 1

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
            width: rowContent.width - meta.width - (root.showDeleteIcon ? 56 : 34)
            anchors.verticalCenter: parent.verticalCenter
            font.family: "IBM Plex Sans, Noto Sans, Sans Serif"
            font.pixelSize: previewFontSize
        }

        Item {
            id: meta
            width: metaRow.implicitWidth
            height: metaRow.implicitHeight
            anchors.verticalCenter: parent.verticalCenter

            Row {
                id: metaRow
                spacing: 6
                anchors.verticalCenter: parent.verticalCenter
                // PIN indicator (if needed)
                visible: pinned
                Item {
                    visible: pinned
                    width: 14
                    height: 14
                    anchors.verticalCenter: parent.verticalCenter

                    Canvas {
                        anchors.fill: parent
                        onPaint: {
                            var ctx = getContext('2d');
                            ctx.clearRect(0, 0, width, height);
                            ctx.strokeStyle = root.metaTextColor;
                            ctx.fillStyle = root.metaTextColor;
                            ctx.lineWidth = 1.6;
                            ctx.lineCap = 'round';

                            // Pin head
                            ctx.beginPath();
                            ctx.arc(width * 0.5, height * 0.28, width * 0.18, 0, Math.PI * 2);
                            ctx.fill();

                            // Pin body
                            ctx.beginPath();
                            ctx.moveTo(width * 0.5, height * 0.42);
                            ctx.lineTo(width * 0.5, height * 0.78);
                            ctx.stroke();

                            // Pin tip
                            ctx.beginPath();
                            ctx.moveTo(width * 0.42, height * 0.78);
                            ctx.lineTo(width * 0.5, height * 0.96);
                            ctx.lineTo(width * 0.58, height * 0.78);
                            ctx.closePath();
                            ctx.fill();
                        }
                    }
                }

            }
        }

        Item {
            id: deleteButton
            visible: root.showDeleteIcon
            width: 18
            height: 18
            anchors.verticalCenter: parent.verticalCenter
            opacity: deleteMouse.containsMouse ? 1.0 : (root.current ? 0.84 : 0.56)
            scale: deleteMouse.containsMouse ? 1.06 : 1.0

            Behavior on opacity {
                NumberAnimation { duration: 120; easing.type: Easing.OutQuad }
            }

            Behavior on scale {
                NumberAnimation { duration: 120; easing.type: Easing.OutQuad }
            }

            Canvas {
                id: deleteCanvas
                anchors.fill: parent
                property bool hot: deleteMouse.containsMouse
                onHotChanged: requestPaint()
                onPaint: {
                    var ctx = getContext('2d');
                    ctx.clearRect(0, 0, width, height);
                    ctx.globalAlpha = hot ? 0.98 : 0.92;
                    ctx.strokeStyle = root.deleteIconColor;
                    ctx.lineWidth = 2;
                    ctx.lineCap = 'round';
                    ctx.beginPath();
                    ctx.moveTo(width*0.3, height*0.3);
                    ctx.lineTo(width*0.7, height*0.7);
                    ctx.moveTo(width*0.7, height*0.3);
                    ctx.lineTo(width*0.3, height*0.7);
                    ctx.stroke();
                }
            }

            MouseArea {
                id: deleteMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    // Pinned items cannot be deleted
                    if (root.pinned) {
                        mouse.accepted = true
                        return
                    }

                    if (listView) {
                        listView.currentIndex = row
                        listView.forceActiveFocus()
                    }

                    if (modelRef)
                        modelRef.deleteAt(row)
                    mouse.accepted = true
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        z: 0
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
