import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qlippy

Window {
    id: root
    width: 760
    height: 480
    visible: false
    color: "transparent"
    property bool optionsOpen: false
    property ClipboardModel clipboardModel
    property SettingsModel settingsModel
    property PopupController popupController
    readonly property var themeData: themeFor(settingsModel.theme)
    readonly property var layoutData: layoutFor(settingsModel.layout)

    SystemPalette {
        id: systemPalette
        colorGroup: SystemPalette.Active
    }

    function mixColor(a, b, amount) {
        const mixAmount = Math.max(0, Math.min(1, amount))
        return Qt.rgba(
            a.r + (b.r - a.r) * mixAmount,
            a.g + (b.g - a.g) * mixAmount,
            a.b + (b.b - a.b) * mixAmount,
            a.a + (b.a - a.a) * mixAmount
        )
    }

    function colorLuma(color) {
        return (0.2126 * color.r) + (0.7152 * color.g) + (0.0722 * color.b)
    }

    function withAlpha(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha)
    }

    function systemTheme(forceDark) {
        const rawBg = systemPalette.window
        const rawBase = systemPalette.base
        const rawText = systemPalette.windowText
        const accent = systemPalette.highlight
        const nativeDark = colorLuma(rawBg) < 0.5
        const isDark = forceDark === undefined ? nativeDark : forceDark

        // If the platform only exposes one palette direction, synthesize the other one.
        const bg = isDark
            ? (nativeDark ? rawBg : mixColor(rawBg, Qt.rgba(0, 0, 0, 1), 0.84))
            : (nativeDark ? mixColor(rawBg, Qt.rgba(1, 1, 1, 1), 0.86) : rawBg)
        const base = isDark
            ? (nativeDark ? rawBase : mixColor(rawBase, Qt.rgba(0, 0, 0, 1), 0.78))
            : (nativeDark ? mixColor(rawBase, Qt.rgba(1, 1, 1, 1), 0.82) : rawBase)
        const text = isDark
            ? (nativeDark ? rawText : mixColor(rawText, Qt.rgba(1, 1, 1, 1), 0.88))
            : (nativeDark ? mixColor(rawText, Qt.rgba(0, 0, 0, 1), 0.90) : rawText)

        const panel = mixColor(bg, base, isDark ? 0.5 : 0.72)
        const border = mixColor(bg, text, isDark ? 0.24 : 0.16)
        const muted = mixColor(text, bg, isDark ? 0.42 : 0.52)
        const selectedPanel = mixColor(panel, accent, isDark ? 0.24 : 0.14)
        const warningSeed = isDark ? Qt.rgba(0.92, 0.76, 0.50, 1) : Qt.rgba(0.70, 0.48, 0.20, 1)
        const warning = mixColor(warningSeed, accent, isDark ? 0.12 : 0.06)
        const veilBase = isDark ? Qt.rgba(0, 0, 0, 1) : Qt.rgba(0.10, 0.12, 0.16, 1)

        return {
            bg: bg,
            panel: panel,
            text: text,
            accent: accent,
            border: border,
            muted: muted,
            warning: warning,
            selectedPanel: selectedPanel,
            veil: withAlpha(veilBase, isDark ? 0.42 : 0.20)
        }
    }

    function themeFor(name) {
        if (name === "system-dark") {
            return systemTheme(true)
        }
        if (name === "system-light") {
            return systemTheme(false)
        }
        if (name === "system") {
            return systemTheme(true)
        }
        if (name === "catppuccin") {
            return {
                bg: "#303446",
                panel: "#292C3C",
                text: "#C6D0F5",
                accent: "#CA9EE6",
                border: "#51576D",
                muted: "#A5ADCE",
                warning: "#E5C890",
                selectedPanel: "#414559",
                veil: "#99232634"
            }
        }
        return {
            bg: "#17272C",
            panel: "#22363D",
            text: "#E9F5F3",
            accent: "#92DDD5",
            border: "#5F8888",
            muted: "#A9CAC7",
            warning: "#E9C39E",
            selectedPanel: "#335259",
            veil: "#6610181B"
        }
    }

    function layoutFor(name) {
        if (name === "compact") {
            return {
                outerMargin: 12,
                rowHeight: 48,
                rowPadding: 10,
                rowFontSize: 13,
                metaFontSize: 11,
                searchFontSize: 14,
                searchHorizontalInset: 12,
                searchVerticalInset: 8,
                detailInset: 8,
                detailHeaderInset: 8,
                detailHeaderGap: 6,
                statusFontSize: 11,
                statusBarHeight: 34,
                controlFontSize: 12,
                labelFontSize: 12,
                sectionFontSize: 11,
                titleFontSize: 16,
                itemSpacing: 6,
                sliderInset: 12,
                sliderSectionGap: 12,
                sliderControlHeight: 30,
                sliderTallControlHeight: 34,
                sliderButtonSize: 22
            }
        }
        if (name === "big") {
            return {
                outerMargin: 16,
                rowHeight: 68,
                rowPadding: 16,
                rowFontSize: 16,
                metaFontSize: 13,
                searchFontSize: 18,
                searchHorizontalInset: 16,
                searchVerticalInset: 12,
                detailInset: 12,
                detailHeaderInset: 12,
                detailHeaderGap: 10,
                statusFontSize: 13,
                statusBarHeight: 44,
                controlFontSize: 14,
                labelFontSize: 14,
                sectionFontSize: 13,
                titleFontSize: 20,
                itemSpacing: 10,
                sliderInset: 16,
                sliderSectionGap: 18,
                sliderControlHeight: 36,
                sliderTallControlHeight: 42,
                sliderButtonSize: 28
            }
        }
        return {
            outerMargin: 14,
            rowHeight: 58,
            rowPadding: 12,
            rowFontSize: 14,
            metaFontSize: 12,
            searchFontSize: 16,
            searchHorizontalInset: 14,
            searchVerticalInset: 10,
            detailInset: 10,
            detailHeaderInset: 10,
            detailHeaderGap: 8,
            statusFontSize: 12,
            statusBarHeight: 38,
            controlFontSize: 13,
            labelFontSize: 13,
            sectionFontSize: 12,
            titleFontSize: 18,
            itemSpacing: 8,
            sliderInset: 14,
            sliderSectionGap: 16,
            sliderControlHeight: 32,
            sliderTallControlHeight: 38,
            sliderButtonSize: 24
        }
    }

    // Clear search box and re-focus when the popup is shown
    Connections {
        target: popupController
        function onResetSearch() {
            searchBox.text = ""
            clipboardModel.searchText = ""
            list.collapseExpanded()
            list.currentIndex = 0
            root.optionsOpen = false
            hoverTracker.resetTracking()
            searchBox.forceActiveFocus()
        }
    }

    Connections {
        target: settingsModel
        function onExpandModeChanged() {
            if (!settingsModel.expandMode)
                list.collapseExpanded()
        }
        function onMaxHistoryChanged() {
            clipboardModel.resultLimit = settingsModel.maxHistory
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: 18
        border.width: 1
        border.color: themeData.border
        opacity: settingsModel.opacity
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.lighter(themeData.bg, 1.06) }
            GradientStop { position: 1.0; color: Qt.darker(themeData.bg, 1.08) }
        }

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: -70
            width: 260
            height: 260
            radius: 130
            color: themeData.accent
            opacity: 0.25
        }

        Rectangle {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins: -80
            width: 300
            height: 300
            radius: 150
            color: themeData.warning
            opacity: 0.15
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: layoutData.outerMargin
            spacing: layoutData.itemSpacing + 2

            SearchBox {
                id: searchBox
                Layout.fillWidth: true
                textColor: themeData.text
                placeholderColor: themeData.muted
                selectionFill: themeData.accent
                selectionText: themeData.bg
                panelColor: themeData.panel
                borderColor: themeData.border
                accentColor: themeData.accent
                fieldFontSize: layoutData.searchFontSize
                horizontalInset: layoutData.searchHorizontalInset
                verticalInset: layoutData.searchVerticalInset
                onTextChanged: clipboardModel.searchText = text
                // Keyboard nav from search box: down arrow moves focus to list
                Keys.onDownPressed: function(event) {
                    if (list.expandedId !== -1)
                        focusExpandedView()
                    else
                        list.forceActiveFocus()
                    event.accepted = true
                }
                Keys.onEscapePressed: function(event) { root.hide(); event.accepted = true }
                Keys.onReturnPressed: function(event) {
                    if (list.count > 0) {
                        list.currentIndex = 0
                        clipboardModel.activate(0)
                    }
                    event.accepted = true
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                ListView {
                    id: list
                    anchors.fill: parent
                    model: clipboardModel
                    property int expandedId: -1
                    property string expandedMode: "text"
                    property string expandedText: ""
                    property string expandedImageUrl: ""
                    spacing: layoutData.itemSpacing
                    clip: true
                    focus: true
                    visible: expandedId === -1
                    currentIndex: count > 0 ? 0 : -1

                    Component.onCompleted: clipboardModel.resultLimit = settingsModel.maxHistory

                    function collapseExpanded() {
                        expandedId = -1
                        expandedMode = "text"
                        expandedText = ""
                        expandedImageUrl = ""
                    }

                    function toggleExpandRow(row) {
                        if (!settingsModel.expandMode)
                            return

                        const id = clipboardModel.idAt(row)
                        if (id <= 0)
                            return

                        if (expandedId === id) {
                            collapseExpanded()
                            forceActiveFocus()
                            return
                        }

                        expandedId = id
                        expandedMode = clipboardModel.typeAt(row)
                        if (expandedMode === "image") {
                            expandedImageUrl = clipboardModel.imageDataUrlAt(row)
                            expandedText = ""
                            detailPanel.adjustImage = settingsModel.compactImageExpand
                        } else {
                            expandedText = clipboardModel.fullTextAt(row)
                            expandedImageUrl = ""
                        }
                        focusExpandedView()
                    }

                    delegate: ClipItemDelegate {
                        width: list.width
                        current: ListView.isCurrentItem
                        row: index
                        listView: list
                        modelRef: clipboardModel
                        itemId: model.id
                        pinned: model.pinned
                        expandEnabled: settingsModel.expandMode
                        previewText: model.preview
                        typeText: model.type
                        rowHeight: layoutData.rowHeight
                        rowPadding: layoutData.rowPadding
                        previewFontSize: layoutData.rowFontSize
                        metaFontSize: layoutData.metaFontSize
                        baseColor: themeData.panel
                        currentColor: themeData.selectedPanel
                        baseBorderColor: themeData.border
                        currentBorderColor: themeData.accent
                        bodyTextColor: themeData.text
                        metaTextColor: themeData.warning
                        textIndicatorColor: themeData.accent
                        imageIndicatorColor: themeData.warning
                        showDeleteIcon: settingsModel.allowDeletionItems
                        deleteIconColor: themeData.warning
                    }

                    Keys.onPressed: function(event) {
                        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                            clipboardModel.activate(currentIndex)
                            event.accepted = true
                        } else if (event.key === Qt.Key_Delete) {
                            if (settingsModel.allowDeletionItems) {
                                // Don't delete pinned items
                                if (!clipboardModel.isPinnedAt(currentIndex)) {
                                    clipboardModel.deleteAt(currentIndex)
                                    event.accepted = true
                                }
                            }
                        } else if (event.key === Qt.Key_P) {
                            clipboardModel.togglePinAt(currentIndex)
                            event.accepted = true
                        } else if (event.key === Qt.Key_Space) {
                            toggleExpandRow(currentIndex)
                            event.accepted = true
                        } else if (event.key === Qt.Key_Escape) {
                            root.hide()
                            event.accepted = true
                        } else if (event.key === Qt.Key_Up && currentIndex === 0) {
                            searchBox.forceActiveFocus()
                            event.accepted = true
                        } else if (event.text.length > 0 && !event.modifiers) {
                            // Printable character — route to search box
                            searchBox.forceActiveFocus()
                            searchBox.text += event.text
                            event.accepted = true
                        }
                    }

                    // Hover selection follows actual mouse movement over the
                    // viewport, not delegate enter events, so keyboard scroll
                    // is not overridden by a stationary pointer.
                    MouseArea {
                        id: hoverTracker
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.NoButton
                        property bool hasBaseline: false
                        property real lastMouseX: 0
                        property real lastMouseY: 0

                        function resetTracking() {
                            hasBaseline = false
                            lastMouseX = 0
                            lastMouseY = 0
                        }

                        onPositionChanged: {
                            if (!hasBaseline) {
                                lastMouseX = mouseX
                                lastMouseY = mouseY
                                hasBaseline = true
                                return
                            }

                            const moved = Math.abs(mouseX - lastMouseX) > 0.1 || Math.abs(mouseY - lastMouseY) > 0.1
                            lastMouseX = mouseX
                            lastMouseY = mouseY
                            if (!moved)
                                return

                            const idx = list.indexAt(mouseX, mouseY + list.contentY)
                            if (idx >= 0 && idx !== list.currentIndex) {
                                list.currentIndex = idx
                                list.forceActiveFocus()
                            }
                        }

                        onExited: resetTracking()
                    }
                }

                Rectangle {
                    id: detailPanel
                    anchors.fill: parent
                    visible: list.expandedId !== -1
                    radius: 12
                    color: Qt.darker(themeData.panel, 1.08)
                    border.width: 1
                    border.color: themeData.border
                    property bool adjustImage: settingsModel.compactImageExpand

                    RowLayout {
                        id: detailHeader
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: layoutData.detailHeaderInset
                        spacing: layoutData.detailHeaderGap

                        Text {
                            Layout.fillWidth: true
                            text: list.expandedMode === "image" ? "Expanded image" : "Expanded entry"
                            color: themeData.accent
                            font.family: "IBM Plex Sans, Noto Sans, Sans Serif"
                            font.pixelSize: layoutData.controlFontSize
                        }

                        CheckBox {
                            id: adjustImageCheck
                            visible: list.expandedMode === "image"
                            checked: detailPanel.adjustImage
                            onCheckedChanged: detailPanel.adjustImage = checked
                            focusPolicy: Qt.NoFocus
                            spacing: 5
                            contentItem: Text {
                                leftPadding: adjustImageCheck.indicator.width + adjustImageCheck.spacing
                                text: "Adjust image"
                                font.family: "IBM Plex Mono, Monospace"
                                font.pixelSize: layoutData.metaFontSize
                                color: themeData.muted
                                verticalAlignment: Text.AlignVCenter
                            }
                            indicator: Rectangle {
                                implicitWidth: layoutData.metaFontSize + 4
                                implicitHeight: layoutData.metaFontSize + 4
                                x: adjustImageCheck.leftPadding
                                y: (adjustImageCheck.height - height) / 2
                                radius: 3
                                color: adjustImageCheck.checked ? themeData.accent : "transparent"
                                border.color: adjustImageCheck.checked ? themeData.accent : themeData.muted
                                border.width: 1
                                Text {
                                    anchors.centerIn: parent
                                    text: "\u2713"
                                    color: themeData.bg
                                    font.pixelSize: parent.implicitHeight - 3
                                    visible: adjustImageCheck.checked
                                }
                            }
                        }

                        Text {
                            text: "Space/Esc=collapse"
                            color: themeData.muted
                            font.family: "IBM Plex Mono, Monospace"
                            font.pixelSize: layoutData.metaFontSize
                        }
                    }

                    ScrollView {
                        id: detailTextScroll
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: detailHeader.bottom
                        anchors.bottom: parent.bottom
                        anchors.margins: layoutData.detailInset
                        visible: list.expandedMode === "text"
                        clip: true
                        ScrollBar.horizontal.policy: ScrollBar.AsNeeded
                        ScrollBar.vertical.policy: ScrollBar.AsNeeded

                        TextArea {
                            id: detailText
                            readOnly: true
                            text: list.expandedText
                            color: themeData.text
                            textFormat: TextEdit.PlainText
                            selectByMouse: true
                            wrapMode: TextEdit.NoWrap
                            selectionColor: themeData.accent
                            selectedTextColor: themeData.bg
                            font.family: "IBM Plex Mono, Monospace"
                            font.pixelSize: layoutData.controlFontSize
                            background: Rectangle {
                                radius: 8
                                color: Qt.darker(themeData.bg, 1.05)
                                border.width: 1
                                border.color: themeData.border
                            }
                            Keys.onPressed: function(event) {
                                if (event.key === Qt.Key_Escape || event.key === Qt.Key_Space) {
                                    list.collapseExpanded()
                                    list.forceActiveFocus()
                                    event.accepted = true
                                }
                            }
                        }
                    }

                    Rectangle {
                        id: imageSurface
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: detailHeader.bottom
                        anchors.bottom: parent.bottom
                        anchors.margins: layoutData.detailInset
                        visible: list.expandedMode === "image"
                        radius: 8
                        color: Qt.darker(themeData.bg, 1.05)
                        border.width: 1
                        border.color: themeData.border

                        Flickable {
                            id: imageFlick
                            anchors.fill: parent
                            anchors.margins: Math.max(4, layoutData.detailInset - 2)
                            clip: true
                            contentWidth: Math.max(width, imagePreview.implicitWidth)
                            contentHeight: Math.max(height, imagePreview.implicitHeight)
                            boundsBehavior: Flickable.StopAtBounds
                            ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }
                            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                            Keys.onPressed: function(event) {
                                if (event.key === Qt.Key_Escape || event.key === Qt.Key_Space) {
                                    list.collapseExpanded()
                                    list.forceActiveFocus()
                                    event.accepted = true
                                }
                            }

                            Image {
                                id: imagePreview
                                source: list.expandedImageUrl
                                asynchronous: true
                                cache: false
                                sourceSize.width: 0
                                sourceSize.height: 0
                                fillMode: detailPanel.adjustImage ? Image.PreserveAspectFit : Image.Pad
                                width: detailPanel.adjustImage ? imageFlick.width : implicitWidth
                                height: detailPanel.adjustImage ? imageFlick.height : implicitHeight
                                x: detailPanel.adjustImage ? 0 : (imageFlick.contentWidth - width) / 2
                                y: detailPanel.adjustImage ? 0 : (imageFlick.contentHeight - height) / 2
                            }

                            Text {
                                anchors.centerIn: parent
                                visible: imagePreview.status === Image.Error || list.expandedImageUrl.length === 0
                                text: "Unable to render image payload"
                                color: themeData.warning
                                font.family: "IBM Plex Sans, Noto Sans, Sans Serif"
                                font.pixelSize: layoutData.controlFontSize
                            }
                        }
                    }
                }
            }

            Preview {
                Layout.fillWidth: true
                Layout.preferredHeight: layoutData.statusBarHeight
                optionsOpen: root.optionsOpen
                panelColor: Qt.darker(themeData.panel, 1.12)
                borderColor: themeData.border
                textColor: themeData.text
                mutedTextColor: themeData.muted
                accentTextColor: themeData.warning
                separatorColor: themeData.border
                statusFontSize: layoutData.statusFontSize
                leftText: 'Vibe-coded by <a href="https://github.com/p0nti/qlippy"><font color="#A8C7E6">p0nti</font></a>'
                centerText: "Version " + (popupController.appVersion.length > 0 ? popupController.appVersion : "0.1.0")
                rightText: "Entries: " + clipboardModel.count
                onOptionsClicked: root.optionsOpen = !root.optionsOpen
            }
        }

        Rectangle {
            anchors.fill: parent
            radius: 18
            color: themeData.veil
            opacity: root.optionsOpen ? 1.0 : 0.0
            visible: root.optionsOpen || opacity > 0.0
            z: 10

            Behavior on opacity {
                NumberAnimation { duration: 140; easing.type: Easing.OutQuad }
            }

            MouseArea {
                anchors.fill: parent
                enabled: root.optionsOpen
                onClicked: root.optionsOpen = false
            }
        }

        OptionsSlider {
            readonly property real sliderMargin: root.layoutData ? root.layoutData.outerMargin - 2 : 12
            y: sliderMargin
            height: parent.height - (sliderMargin * 2)
            panelOpen: root.optionsOpen
            panelWidth: Math.floor(parent.width * 0.35)
            settingsObject: settingsModel
            themeData: root.themeData
            layoutData: root.layoutData
            z: 11
            onCloseRequested: root.optionsOpen = false
        }
    }

    function focusExpandedView() {
        if (list.expandedMode === "image")
            imageFlick.forceActiveFocus()
        else
            detailText.forceActiveFocus()
    }

    Component.onCompleted: {
        clipboardModel.refresh()
        searchBox.forceActiveFocus()
    }
}
