import QtQuick 2.6

// Press and hold to look, for as long as the finger stays down: the last
// trick, laid under the table's cards.
MouseArea {
    property bool peeking

    // Below the cards and the tap guard.
    z: 0.4
    onPressAndHold: peeking = true
    onReleased: peeking = false
    onCanceled: peeking = false
    onEnabledChanged: peeking = false
}
