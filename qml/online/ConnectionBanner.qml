import QtQuick 2.6
import Sailfish.Silica 1.0

// Across the top of the game while the connection to an online table is lost:
// moves can't be made until it is back.
Rectangle {
    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
    }
    height: label.height + 2 * Theme.paddingMedium
    z: 100
    visible: online.started && !online.synced
    color: Theme.rgba(Theme.highlightDimmerColor, 0.9)

    Label {
        id: label

        anchors.centerIn: parent
        width: parent.width - 2 * Theme.horizontalPageMargin
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
        text: qsTr("Connection lost, reconnecting…")
        color: Theme.highlightColor
        font.pixelSize: Theme.fontSizeSmall
    }
}
