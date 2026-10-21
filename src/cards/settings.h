#pragma once

// Where a game keeps its settings and statistics. Qt only.

#include <QCoreApplication>
#include <QStandardPaths>
#include <QString>

namespace cards {

// The app's own config folder: sailjail lets an app write only there.
inline QString settingsPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QLatin1Char('/')
           + QCoreApplication::applicationName() + QStringLiteral(".conf");
}

} // namespace cards
