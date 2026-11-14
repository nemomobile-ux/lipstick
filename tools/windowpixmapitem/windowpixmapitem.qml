import QtQuick 2.6
import QtQuick.Window 2.2
import org.nemomobile.lipstick 0.1

Window {
    id: root
    visible: true
    width: 640
    height: 480
    title: "WindowPixmapItem test"
    color: "#333333"

    property int selectedWindowId: (typeof windowIdOverride !== "undefined" ? windowIdOverride : 0)

    WindowModel {
        id: windowModel
    }

    function pickFirst() {
        if (windowModel.itemCount > 0 && selectedWindowId === 0) {
            selectedWindowId = windowModel.windowId(0)
        }
    }

    Connections {
        target: windowModel
        function onItemCountChanged() { pickFirst() }
    }

    Component.onCompleted: pickFirst()

    Row {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        Column {
            width: 220
            spacing: 6

            Text {
                text: "Windows: " + windowModel.itemCount
                color: "white"
            }

            Repeater {
                model: windowModel
                delegate: Rectangle {
                    width: 220
                    height: 28
                    color: root.selectedWindowId === model.window ? "#4466aa" : "#555555"
                    radius: 4
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 6
                        color: "white"
                        text: model.window + (model.title ? " - " + model.title : "")
                        elide: Text.ElideRight
                        width: parent.width - 12
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.selectedWindowId = model.window
                    }
                }
            }

            Text {
                color: "#cccccc"
                width: 220
                wrapMode: Text.WordWrap
                text: "Run inside a Lipstick compositor to preview real windows."
            }
        }

        Rectangle {
            id: previewBg
            width: parent.width - 232
            height: parent.height
            color: "#222222"
            radius: 8

            Rectangle {
                id: greenBg
                anchors.fill: parent
                anchors.margins: 20
                color: "green"
                radius: 16
            }

            WindowPixmapItem {
                id: pixmap
                anchors.fill: greenBg
                windowId: root.selectedWindowId
                radius: 16
                smooth: true
            }

            Column {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: 10
                spacing: 2
                Text { color: "white"; text: "windowId: " + pixmap.windowId }
                Text { color: "white"; text: "hasPixmap: " + pixmap.hasPixmap }
                Text { color: "white"; text: "windowSize: " + pixmap.windowSize.width + " x " + pixmap.windowSize.height }
            }
        }
    }
}