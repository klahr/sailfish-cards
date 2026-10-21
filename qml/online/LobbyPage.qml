import QtQuick 2.6
import Sailfish.Silica 1.0

// The games on the server: join one, or set up your own. Once you sit down,
// the table takes the lobby's place, or the game itself if it has begun.
Page {
    id: page

    function seated() {
        if (status !== PageStatus.Active || !online.atTable)
            return
        if (online.started)
            pageStack.pop()
        else
            pageStack.replace(Qt.resolvedUrl("TablePage.qml"))
    }

    function title(game) {
        if (game.title)
            return game.title
        for (var i = 0; i < game.players.length; ++i)
            if (game.players[i])
                return qsTr("%1's game").arg(game.players[i])
        return qsTr("Game")
    }

    function players(game) {
        return game.players.map(function(name) { return name || "–" }).join(", ")
    }

    // A seat no one has taken, or a bot's in a game under way.
    function canJoin(game) {
        return game.mine || game.players.indexOf("") >= 0
    }

    Component.onCompleted: online.connectToServer()
    // Not at a table: no need to stay connected.
    Component.onDestruction: if (!online.atTable) online.disconnectFromServer()
    onStatusChanged: seated()

    Connections {
        target: online
        onTableChanged: page.seated()
    }

    SilicaListView {
        id: list

        anchors.fill: parent
        model: online.games

        PullDownMenu {
            MenuItem {
                text: qsTr("Connect again")
                visible: online.status === 0
                onClicked: online.connectToServer()
            }
            MenuItem {
                text: qsTr("New game")
                enabled: online.status === 2
                onClicked: pageStack.push(newGameDialog)
            }
        }

        header: Column {
            width: list.width

            PageHeader {
                title: qsTr("Play online")
                description: online.status === 2 ? qsTr("Connected")
                                                 : online.status === 1 ? qsTr("Connecting…") : qsTr("Not connected")
            }
            TextField {
                width: parent.width
                label: qsTr("Your name")
                placeholderText: label
                text: online.name
                EnterKey.iconSource: "image://theme/icon-m-enter-close"
                EnterKey.onClicked: focus = false
                onActiveFocusChanged: if (!activeFocus) online.name = text
            }
            TextField {
                width: parent.width
                label: qsTr("Server")
                placeholderText: label
                text: online.server
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhUrlCharactersOnly
                EnterKey.iconSource: "image://theme/icon-m-enter-close"
                EnterKey.onClicked: focus = false
                onActiveFocusChanged: if (!activeFocus) online.server = text
            }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                visible: text !== ""
                text: online.error
                color: Theme.errorColor
                wrapMode: Text.Wrap
            }
            SectionHeader {
                text: qsTr("Games")
                visible: list.count > 0
            }
        }

        delegate: ListItem {
            id: item

            readonly property bool joinable: page.canJoin(modelData)

            contentHeight: Theme.itemSizeMedium
            enabled: joinable
            opacity: joinable ? 1 : Theme.opacityLow
            onClicked: {
                if (modelData.locked && !modelData.mine)
                    pageStack.push(passwordDialog, { gameId: modelData.id })
                else
                    online.join(modelData.id)
            }

            Column {
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    right: lock.left
                    rightMargin: Theme.paddingMedium
                    verticalCenter: parent.verticalCenter
                }

                Label {
                    width: parent.width
                    text: page.title(modelData)
                    color: item.highlighted ? Theme.highlightColor : Theme.primaryColor
                    truncationMode: TruncationMode.Fade
                }
                Label {
                    width: parent.width
                    text: (modelData.mine ? qsTr("Yours") : modelData.started ? qsTr("Playing") : qsTr("Waiting"))
                          + " · " + page.players(modelData)
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: item.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                    truncationMode: TruncationMode.Fade
                }
            }
            Icon {
                id: lock

                anchors {
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                source: "image://theme/icon-s-secure"
                visible: modelData.locked
            }
        }

        ViewPlaceholder {
            enabled: list.count === 0 && online.status === 2
            text: qsTr("No games")
            hintText: qsTr("Pull down to start one")
        }

        VerticalScrollDecorator { }
    }

    BusyIndicator {
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
        running: online.status === 1
    }

    Component {
        id: newGameDialog

        Dialog {
            canAccept: online.status === 2
            onAccepted: online.create(titleField.text, passwordField.text, online.minSeats + seatsBox.currentIndex)

            Column {
                width: parent.width

                DialogHeader { acceptText: qsTr("Create") }
                TextField {
                    id: titleField

                    width: parent.width
                    label: qsTr("Name of the game")
                    placeholderText: qsTr("%1's game").arg(online.name)
                    EnterKey.iconSource: "image://theme/icon-m-enter-next"
                    EnterKey.onClicked: passwordField.focus = true
                }
                PasswordField {
                    id: passwordField

                    width: parent.width
                    label: qsTr("Password, if you want one")
                    placeholderText: qsTr("No password")
                    EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                    EnterKey.onClicked: accept()
                }
                // For a game that several numbers of players can play.
                ComboBox {
                    id: seatsBox

                    visible: online.maxSeats > online.minSeats
                    label: qsTr("Players")
                    menu: ContextMenu {
                        Repeater {
                            model: online.maxSeats - online.minSeats + 1
                            MenuItem { text: online.minSeats + index }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: passwordDialog

        Dialog {
            property string gameId

            canAccept: password.text !== ""
            onAccepted: online.join(gameId, password.text)

            Column {
                width: parent.width

                DialogHeader { acceptText: qsTr("Join") }
                PasswordField {
                    id: password

                    width: parent.width
                    focus: true
                    EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                    EnterKey.onClicked: accept()
                }
            }
        }
    }
}
