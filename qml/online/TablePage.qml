import QtQuick 2.6
import Sailfish.Silica 1.0

// Who sits at your table. Before the game the creator starts it, with bots in
// the empty seats; during it, they can let a bot play for someone who is away.
Page {
    id: page

    // Opened before the start: when it comes, the game takes over.
    property bool waiting

    function update() {
        if (status !== PageStatus.Active)
            return
        if (!online.atTable)
            pageStack.pop()
        else if (waiting && online.started)
            pageStack.pop()
    }

    Component.onCompleted: waiting = !online.started
    // Not at a table: no need to stay connected.
    Component.onDestruction: if (!online.atTable) online.disconnectFromServer()
    onStatusChanged: update()

    Connections {
        target: online
        onTableChanged: page.update()
    }

    SilicaListView {
        id: list

        anchors.fill: parent
        model: online.seats

        PullDownMenu {
            MenuItem {
                text: qsTr("Leave the table")
                onClicked: online.leave()
            }
        }

        header: Column {
            width: list.width

            PageHeader {
                title: online.title || qsTr("Your table")
                description: online.started ? qsTr("Playing") : qsTr("Waiting for players")
            }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                visible: text !== ""
                text: online.status !== 2 ? qsTr("The connection is lost, trying again…") : online.error
                color: Theme.errorColor
                wrapMode: Text.Wrap
            }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                visible: online.started && online.admin
                text: qsTr("Long press someone who is away to let a bot play for them until they are back.")
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryHighlightColor
                wrapMode: Text.Wrap
                bottomPadding: Theme.paddingMedium
            }
        }

        delegate: ListItem {
            id: item

            readonly property bool away: modelData.human && !modelData.online

            contentHeight: Theme.itemSizeSmall
            menu: online.admin && online.started && away ? botMenu : null

            Label {
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    right: stateLabel.left
                    rightMargin: Theme.paddingMedium
                    verticalCenter: parent.verticalCenter
                }
                text: modelData.human ? (modelData.name || qsTr("Player %1").arg(index + 1))
                                      : online.started ? qsTr("Bot") : qsTr("Empty seat")
                color: index === online.seat || item.highlighted ? Theme.highlightColor : Theme.primaryColor
                truncationMode: TruncationMode.Fade
            }
            Label {
                id: stateLabel

                anchors {
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                text: index === online.seat ? qsTr("You")
                                            : !modelData.human ? ""
                                            : modelData.online ? "" : modelData.bot ? qsTr("Away, bot plays") : qsTr("Away")
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
            }

            Component {
                id: botMenu

                ContextMenu {
                    MenuItem {
                        text: modelData.bot ? qsTr("Wait for them") : qsTr("Let a bot play")
                        onClicked: online.setBot(index, !modelData.bot)
                    }
                }
            }
        }

        footer: Column {
            width: list.width

            Item {
                width: parent.width
                height: start.visible ? start.height + 2 * Theme.paddingLarge : 0

                Button {
                    id: start

                    anchors.centerIn: parent
                    visible: online.admin && !online.started
                    text: qsTr("Start")
                    onClicked: online.start()
                }
            }
            // Players in the lobby, to ask to the empty seats.
            SectionHeader {
                text: qsTr("Invite players")
                visible: !online.started
            }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                visible: !online.started && online.players.length === 0
                text: qsTr("No one else is in the lobby.")
                color: Theme.secondaryHighlightColor
                wrapMode: Text.Wrap
                bottomPadding: Theme.paddingMedium
            }
            Repeater {
                model: online.started ? [] : online.players

                BackgroundItem {
                    id: player

                    readonly property bool invited: online.invited.indexOf(modelData.id) >= 0

                    width: parent.width
                    enabled: !invited
                    onClicked: online.invite(modelData.id)

                    Label {
                        anchors {
                            left: parent.left
                            leftMargin: Theme.horizontalPageMargin
                            right: invitedLabel.left
                            rightMargin: Theme.paddingMedium
                            verticalCenter: parent.verticalCenter
                        }
                        text: modelData.name || qsTr("Player")
                        color: player.highlighted ? Theme.highlightColor : Theme.primaryColor
                        truncationMode: TruncationMode.Fade
                    }
                    Label {
                        id: invitedLabel

                        anchors {
                            right: parent.right
                            rightMargin: Theme.horizontalPageMargin
                            verticalCenter: parent.verticalCenter
                        }
                        text: player.invited ? qsTr("Invited") : qsTr("Invite")
                        font.pixelSize: Theme.fontSizeSmall
                        color: player.invited ? Theme.secondaryColor : Theme.highlightColor
                    }
                }
            }
        }

        VerticalScrollDecorator { }
    }
}
