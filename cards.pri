# Shared code for the Sailfish card games. In a game's .pro, after TARGET:
#
#   include(../sailfish-cards/cards.pri)
#
# C++: #include "cards/card.h". QML: import "../cards" for PlayingCard and
# import "../cards/cards.js" as Cards, from a page in the game's qml/pages.

CARDS_DIR = $$PWD

INCLUDEPATH += $$PWD/src
# Not card.cpp: qmake puts all objects in one directory, and games have their own.
SOURCES += $$PWD/src/cards/cards.cpp
HEADERS += $$PWD/src/cards/card.h \
    $$PWD/src/cards/testing.h

# Installed into the game's own qml directory, as qml/cards.
cards_qml.files = $$PWD/qml/PlayingCard.qml $$PWD/qml/cards.js
cards_qml.path = /usr/share/$${TARGET}/qml/cards
INSTALLS += cards_qml

DISTFILES += $$PWD/qml/PlayingCard.qml \
    $$PWD/qml/cards.js \
    $$PWD/README.md
