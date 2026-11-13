/*
 * Example QML for testing WindowPixmapItem.
 */

import QtQuick 2.6
import QtQuick.Window 2.2
import org.nemomobile.lipstick 0.1

Window {
    visible: true
    width: 540
    height: 960
    title: "WindowPixmapItem test"
    color: "gray"

    property int testWindowId: windowIdOverride

    Rectangle {
        id: pixmapBackground
        anchors.fill: parent
        anchors.margins: 40
        color: "green"
    }

    WindowPixmapItem {
        id: windowPixmap
        anchors.fill: pixmapBackground
        windowId: testWindowId
        opaque: true
        radius: 16
        xOffset: 0
        yOffset: 0
        xScale: 1
        yScale: 1
        smooth: true
    }

    Rectangle {
        id: testWindow
        x: 40
        y: 40
        width: 200
        height: 120
        color: "green"
        border.color: "white"

        Text {
            anchors.centerIn: parent
            text: "Test window"
            color: "white"
            font.pixelSize: 16
        }
    }

    Column {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 8

        Text {
            text: "hasPixmap: " + windowPixmap.hasPixmap
            color: "white"
        }

        Text {
            text: "windowSize: " + windowPixmap.windowSize
            color: "white"
        }
    }
}