#include "gamelog.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace cards {

GameLog::GameLog()
    // In the app's own folder: sailjail lets the app write only there.
    : m_path(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/game.log"))
{
}

QVariantList GameLog::load() const
{
    QVariantList moves;
    QFile file(m_path);
    if (!file.open(QIODevice::ReadOnly))
        return moves;
    // One move a line: a line cut short by a crash spoils only the game.
    while (!file.atEnd()) {
        const QByteArray line = file.readLine().trimmed();
        if (line.isEmpty())
            continue;
        const QJsonDocument doc = QJsonDocument::fromJson(line);
        if (!doc.isObject())
            return QVariantList();
        moves << doc.object().toVariantMap();
    }
    return moves;
}

void GameLog::append(const QVariantMap &move)
{
    QDir().mkpath(QFileInfo(m_path).path());
    QFile file(m_path);
    if (!file.open(QIODevice::Append))
        return;
    file.write(QJsonDocument(QJsonObject::fromVariantMap(move)).toJson(QJsonDocument::Compact) + '\n');
}

void GameLog::clear()
{
    QFile::remove(m_path);
}

} // namespace cards
