#pragma once

#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace cards {

// The moves of a game against the bots, written down as they are made, so
// that the game carries on where it was when the app comes back. Qt only.
class GameLog
{
public:
    GameLog();

    // The moves so far, in order; none if there are none, or the file is
    // damaged.
    QVariantList load() const;
    void append(const QVariantMap &move);
    // For a new game.
    void clear();

private:
    QString m_path;
};

} // namespace cards
