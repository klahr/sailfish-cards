# Shared code for the Sailfish card games. In a game's .pro, after TARGET:
#
#   include(../sailfish-cards/cards.pri)
#
# C++: #include "cards/card.h", and in Qt "cards/variant.h" for card ids,
# "cards/settings.h" for where the settings go and "cards/gamelog.h" to carry
# a game on after the app closes.
# QML: import "../cards" for PlayingCard, TableCard, TapToPlay, PeekArea,
# ChoiceArea and CallButton, and
# import "../cards/cards.js" as Cards, from a page in the game's qml/pages.

CARDS_DIR = $$PWD

# Its texts go into the game's translations along with the game's own.
TRANSLATION_SOURCES += $$PWD/src $$PWD/qml

INCLUDEPATH += $$PWD/src
# Not card.cpp: qmake puts all objects in one directory, and games have their own.
SOURCES += $$PWD/src/cards/cards.cpp \
    $$PWD/src/cards/gamelog.cpp
HEADERS += $$PWD/src/cards/card.h \
    $$PWD/src/cards/testing.h \
    $$PWD/src/cards/variant.h \
    $$PWD/src/cards/settings.h \
    $$PWD/src/cards/gamelog.h

# Installed into the game's own qml directory, as qml/cards.
cards_qml.files = $$PWD/qml/PlayingCard.qml $$PWD/qml/TableCard.qml $$PWD/qml/TapToPlay.qml $$PWD/qml/PeekArea.qml $$PWD/qml/ChoiceArea.qml $$PWD/qml/CallButton.qml $$PWD/qml/cards.js
cards_qml.path = /usr/share/$${TARGET}/qml/cards
INSTALLS += cards_qml

DISTFILES += $$PWD/qml/PlayingCard.qml \
    $$PWD/qml/TableCard.qml \
    $$PWD/qml/TapToPlay.qml \
    $$PWD/qml/PeekArea.qml \
    $$PWD/qml/ChoiceArea.qml \
    $$PWD/qml/CallButton.qml \
    $$PWD/qml/cards.js \
    $$PWD/README.md
