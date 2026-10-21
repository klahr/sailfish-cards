import QtQuick 2.6
import Sailfish.Silica 1.0

// A card that glides to wherever the game puts it. `place` holds x, y, z,
// rot (degrees), down (face down), hidden, and delay: how long to wait
// before setting off, so dealt cards fly out one after the other. The backs
// glow in the ambience's highlight colour.
PlayingCard {
    id: card

    property var place
    property int moveDelay
    property bool ready

    function moveToPlace() {
        moveDelay = place.delay || 0
        x = place.x
        y = place.y
        rotation = place.rot || 0
    }

    backGlow: Theme.highlightColor
    faceDown: place.down === true
    z: place.z
    opacity: place.hidden === true ? 0 : 1

    onPlaceChanged: moveToPlace()
    Component.onCompleted: {
        moveToPlace()
        ready = true
    }

    Behavior on x {
        enabled: card.ready
        SequentialAnimation {
            PauseAnimation { duration: card.moveDelay }
            NumberAnimation { duration: 320; easing.type: Easing.OutCubic }
        }
    }
    Behavior on y {
        enabled: card.ready
        SequentialAnimation {
            PauseAnimation { duration: card.moveDelay }
            NumberAnimation { duration: 320; easing.type: Easing.OutCubic }
        }
    }
    Behavior on opacity { FadeAnimation { } }
    Behavior on rotation {
        enabled: card.ready
        SequentialAnimation {
            PauseAnimation { duration: card.moveDelay }
            NumberAnimation { duration: 320; easing.type: Easing.OutCubic }
        }
    }
}
