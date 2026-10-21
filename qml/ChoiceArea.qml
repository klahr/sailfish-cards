import QtQuick 2.6
import Sailfish.Silica 1.0

// Where the cards you choose to give away gather before they go: a frame in
// the receiver's colour round `area` ({ x, y, width, height }), with a hint
// while it is empty. The game lays the chosen cards out inside it.
Item {
    id: root

    property var area
    property color color: Theme.highlightColor
    property string text
    property bool empty: true
    // Choosing now.
    property bool shown

    x: area.x - Theme.paddingSmall
    y: area.y - Theme.paddingSmall
    width: area.width + 2 * Theme.paddingSmall
    height: area.height + 2 * Theme.paddingSmall
    opacity: shown ? 1 : 0
    visible: opacity > 0
    Behavior on opacity { FadeAnimation { } }

    Rectangle {
        anchors.fill: parent
        radius: Theme.paddingMedium
        color: Theme.rgba(root.color, 0.08)
        border.color: Theme.rgba(root.color, 0.5)
        border.width: Math.max(1, Math.round(Theme.pixelRatio))
    }

    Label {
        anchors.centerIn: parent
        width: parent.width - 2 * Theme.paddingSmall
        opacity: root.empty ? 1 : 0
        Behavior on opacity { FadeAnimation { } }
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
        text: root.text
        color: Theme.secondaryHighlightColor
        font.pixelSize: Theme.fontSizeLarge
        font.family: Theme.fontFamilyHeading
    }
}
