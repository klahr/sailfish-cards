import QtQuick 2.6

// Guards against playing a card by accident: with `twice`, the first tap on a
// card raises it and the second plays it, and a tap anywhere else on the
// table lowers it again. Put it in the table under the cards, ask tap()
// before playing, and show `raised` as selected.
MouseArea {
    id: root

    property bool twice
    // The card raised by a first tap, -1 for none.
    property int raised: -1

    // Whether this tap plays the card; if not, it is raised instead.
    function tap(cardId) {
        if (!twice || raised === cardId) {
            raised = -1
            return true
        }
        raised = cardId
        return false
    }
    // Lowers the card, if it is the raised one: for a card that may no
    // longer be played.
    function lower(cardId) {
        if (raised === cardId)
            raised = -1
    }

    anchors.fill: parent
    // Below the cards, above the rest of the table.
    z: 0.5
    enabled: raised >= 0
    onClicked: raised = -1
    onTwiceChanged: raised = -1
}
