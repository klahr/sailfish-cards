#pragma once

// Cards as QML and the online log carry them: lists of card ids. Qt only, so
// it stays out of the plain C++ rule tests.

#include "card.h"

#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include <vector>

namespace cards {

inline QVariantList ids(const Cards &cards)
{
    QVariantList out;
    for (const Card &c : cards)
        out << c.id();
    return out;
}

// Cards from a list of ids, each card at most once; false on anything else.
inline bool cardsFrom(const QVariant &list, Cards *out)
{
    bool seen[52] = {};
    out->clear();
    for (const QVariant &v : list.toList()) {
        bool ok = false;
        const int id = v.toInt(&ok);
        if (!ok || id < 0 || id >= 52 || seen[id])
            return false;
        seen[id] = true;
        out->push_back(Card::fromId(id));
    }
    return true;
}

// The whole pack, each card once.
inline bool deckFrom(const QVariant &list, Cards *out)
{
    return cardsFrom(list, out) && out->size() == 52;
}

// The last trick gathered, card by card as it was played, each with who
// played it and whether it won. `played` holds the deal's plays in order,
// the trick on the table included; the winner leads the next trick.
template <typename Play>
QVariantList lastTrick(const std::vector<Play> &played, int tricks, int players, int winner)
{
    QVariantList out;
    for (int i = (tricks - 1) * players; tricks > 0 && i < tricks * players && i < static_cast<int>(played.size()); ++i) {
        QVariantMap m;
        m.insert(QStringLiteral("card"), played[i].card.id());
        m.insert(QStringLiteral("player"), played[i].player);
        m.insert(QStringLiteral("won"), played[i].player == winner);
        out << m;
    }
    return out;
}

} // namespace cards
