# Online play through cards-server, shared by the card games. In a game's .pro:
#
#   include($$PWD/sailfish-cards/online.pri)
#
# C++: #include "online/session.h". QML: pageStack.push(Qt.resolvedUrl("../online/LobbyPage.qml"))
# from a page in the game's qml/pages, with the Session as the `online` context property,
# and `import "../online"` for a ConnectionBanner on the game page.

# The server a new player connects to; they can change it in the lobby.
isEmpty(ONLINE_SERVER): ONLINE_SERVER = cards.r8.rs:7777
DEFINES += ONLINE_SERVER=\\\"$$ONLINE_SERVER\\\"

QT += network
INCLUDEPATH += $$PWD/src
SOURCES += $$PWD/src/online/session.cpp
HEADERS += $$PWD/src/online/session.h

# Installed into the game's own qml directory, as qml/online.
online_qml.files = $$PWD/qml/online/ConnectionBanner.qml $$PWD/qml/online/LobbyPage.qml $$PWD/qml/online/TablePage.qml
online_qml.path = /usr/share/$${TARGET}/qml/online
INSTALLS += online_qml

DISTFILES += $$PWD/qml/online/ConnectionBanner.qml \
    $$PWD/qml/online/LobbyPage.qml \
    $$PWD/qml/online/TablePage.qml
