#include "session.h"

#include "cards/settings.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace {

const int PingMs = 15000;
// The server answers every ping, so this long without a word means the line is dead.
const int SilenceMs = 40000;
const int ConnectMs = 10000;
// Waits between attempts, doubling up to the longest.
const int RetryMs = 2000;
const int MaxRetryMs = 30000;

} // namespace

Session::Session(const QString &game, int seats, int maxSeats, QObject *parent)
    : QObject(parent)
    , m_game(game)
    , m_minSeats(seats)
    , m_maxSeats(qMax(seats, maxSeats))
    , m_settings(cards::settingsPath(), QSettings::IniFormat)
    , m_retryMs(RetryMs)
{
    m_name = m_settings.value(QStringLiteral("online/name"), tr("Player")).toString();
    m_server = m_settings.value(QStringLiteral("online/server"), QStringLiteral(ONLINE_SERVER)).toString();
    m_token = m_settings.value(QStringLiteral("online/token")).toString();
    m_listed = m_settings.value(QStringLiteral("online/listed"), true).toBool();
    // At a table when the app last closed: sit down again.
    m_tableId = m_settings.value(QStringLiteral("online/table")).toString();

    m_ping.setInterval(PingMs);
    connect(&m_ping, &QTimer::timeout, this, [this]() {
        write({{QStringLiteral("t"), QStringLiteral("ping")}});
    });
    m_retry.setSingleShot(true);
    connect(&m_retry, &QTimer::timeout, this, &Session::connectToServer);
    m_watchdog.setSingleShot(true);
    connect(&m_watchdog, &QTimer::timeout, this, &Session::drop);
    // synced() only holds at a table whose game has begun.
    connect(this, &Session::tableChanged, this, &Session::syncedChanged);

    connect(&m_socket, &QTcpSocket::connected, this, [this]() {
        write({{QStringLiteral("t"), QStringLiteral("hello")},
               {QStringLiteral("game"), m_game},
               {QStringLiteral("name"), m_name},
               {QStringLiteral("token"), m_token},
               {QStringLiteral("hidden"), !m_listed},
               {QStringLiteral("v"), ONLINE_VERSION}});
        m_ping.start();
        m_watchdog.start(SilenceMs);
    });
    connect(&m_socket, &QTcpSocket::readyRead, this, &Session::onReadyRead);
    connect(&m_socket, &QTcpSocket::disconnected, this, &Session::onDisconnected);
    connect(&m_socket, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
            this, [this](QAbstractSocket::SocketError) {
                setError(m_socket.errorString());
                if (m_socket.state() == QAbstractSocket::UnconnectedState)
                    onDisconnected();
            });

    if (!m_tableId.isEmpty())
        connectToServer();
}

Session::~Session()
{
    // The socket says it is disconnected as it goes, after the timers are gone.
    m_socket.disconnect(this);
    m_socket.abort();
}

void Session::setName(const QString &name)
{
    const QString trimmed = name.simplified().left(24);
    if (trimmed == m_name)
        return;
    m_name = trimmed;
    m_settings.setValue(QStringLiteral("online/name"), m_name);
    write({{QStringLiteral("t"), QStringLiteral("name")}, {QStringLiteral("name"), m_name}});
    emit nameChanged();
}

void Session::setListed(bool listed)
{
    if (listed == m_listed)
        return;
    m_listed = listed;
    m_settings.setValue(QStringLiteral("online/listed"), m_listed);
    write({{QStringLiteral("t"), QStringLiteral("visible")}, {QStringLiteral("on"), m_listed}});
    emit listedChanged();
}

void Session::setServer(const QString &server)
{
    const QString trimmed = server.trimmed();
    if (trimmed == m_server)
        return;
    m_server = trimmed;
    m_settings.setValue(QStringLiteral("online/server"), m_server);
    emit serverChanged();
    if (m_wanted) {
        disconnectFromServer();
        connectToServer();
    }
}

int Session::toLocal(int seat) const
{
    if (!active())
        return seat;
    const int n = seatCount();
    if (seat < 0 || seat >= n)
        return -1;
    return (seat - m_seat + n) % n;
}

int Session::toTable(int player) const
{
    return active() ? (player + m_seat) % seatCount() : player;
}

bool Session::isBot(int seat) const
{
    return seat >= 0 && seat < m_seats.size()
           && m_seats.at(seat).toMap().value(QStringLiteral("bot")).toBool();
}

bool Session::away(int seat) const
{
    if (!active() || seat < 0 || seat >= m_seats.size())
        return false;
    const QVariantMap s = m_seats.at(seat).toMap();
    return s.value(QStringLiteral("human")).toBool() && !s.value(QStringLiteral("online")).toBool()
           && !s.value(QStringLiteral("bot")).toBool();
}

QString Session::playerName(int seat) const
{
    if (seat < 0 || seat >= m_seats.size())
        return QString();
    const QVariantMap s = m_seats.at(seat).toMap();
    QString name = s.value(QStringLiteral("name")).toString();
    if (!s.value(QStringLiteral("human")).toBool())
        return tr("Bot %1").arg(seat + 1);
    if (name.isEmpty())
        name = tr("Player %1").arg(seat + 1);
    if (!s.value(QStringLiteral("online")).toBool())
        //: A player who has lost the connection; the bot plays meanwhile if `bot`
        return s.value(QStringLiteral("bot")).toBool() ? tr("%1 (bot)").arg(name) : tr("%1 (away)").arg(name);
    return name;
}

void Session::send(const QVariantMap &action)
{
    if (synced())
        write({{QStringLiteral("t"), QStringLiteral("act")}, {QStringLiteral("a"), action}});
}

void Session::connectToServer()
{
    m_wanted = true;
    m_retry.stop();
    if (m_socket.state() != QAbstractSocket::UnconnectedState)
        return;
    const int colon = m_server.lastIndexOf(QLatin1Char(':'));
    const QString host = colon > 0 ? m_server.left(colon) : m_server;
    const quint16 port = colon > 0 ? m_server.mid(colon + 1).toUShort() : 7777;
    setError(QString());
    setStatus(Connecting);
    m_watchdog.start(ConnectMs);
    m_socket.connectToHost(host, port ? port : 7777);
}

void Session::disconnectFromServer()
{
    m_wanted = false;
    m_retry.stop();
    m_watchdog.stop();
    m_socket.abort();
    m_ping.stop();
    setStatus(Offline);
    clearTable();
    m_games.clear();
    m_players.clear();
    emit playersChanged();
    declineInvitation();
    emit gamesChanged();
}

void Session::create(const QString &title, const QString &password, int seats)
{
    write({{QStringLiteral("t"), QStringLiteral("create")},
           {QStringLiteral("seats"), seats == 0 ? m_minSeats : qBound(m_minSeats, seats, m_maxSeats)},
           {QStringLiteral("title"), title},
           {QStringLiteral("password"), password}});
}

void Session::join(const QString &id, const QString &password)
{
    write({{QStringLiteral("t"), QStringLiteral("join")},
           {QStringLiteral("id"), id},
           {QStringLiteral("password"), password}});
}

void Session::leave()
{
    write({{QStringLiteral("t"), QStringLiteral("leave")}});
    clearTable();
}

void Session::start()
{
    write({{QStringLiteral("t"), QStringLiteral("start")}});
}

void Session::setBot(int seat, bool on)
{
    write({{QStringLiteral("t"), QStringLiteral("bot")}, {QStringLiteral("seat"), seat}, {QStringLiteral("on"), on}});
}

void Session::write(const QVariantMap &message)
{
    if (m_socket.state() != QAbstractSocket::ConnectedState)
        return;
    m_socket.write(QJsonDocument(QJsonObject::fromVariantMap(message)).toJson(QJsonDocument::Compact));
    m_socket.write("\n");
}

void Session::onReadyRead()
{
    m_watchdog.start(SilenceMs);
    while (m_socket.canReadLine()) {
        const QJsonDocument doc = QJsonDocument::fromJson(m_socket.readLine());
        if (doc.isObject())
            receive(doc.object().toVariantMap());
    }
}

void Session::receive(const QVariantMap &message)
{
    const QString t = message.value(QStringLiteral("t")).toString();
    if (t == QLatin1String("welcome")) {
        m_token = message.value(QStringLiteral("token")).toString();
        m_settings.setValue(QStringLiteral("online/token"), m_token);
        m_retryMs = RetryMs;
        setStatus(Online);
        // Back after the network dropped: sit down again.
        if (!m_tableId.isEmpty())
            join(m_tableId);
    } else if (t == QLatin1String("lobby")) {
        m_games = message.value(QStringLiteral("games")).toList();
        emit gamesChanged();
    } else if (t == QLatin1String("table")) {
        const bool wasActive = active();
        m_tableId = message.value(QStringLiteral("id")).toString();
        m_settings.setValue(QStringLiteral("online/table"), m_tableId);
        m_title = message.value(QStringLiteral("title")).toString();
        m_started = message.value(QStringLiteral("started")).toBool();
        m_locked = message.value(QStringLiteral("locked")).toBool();
        m_seat = message.value(QStringLiteral("seat")).toInt();
        m_driver = message.value(QStringLiteral("driver")).toInt();
        m_admin = message.value(QStringLiteral("admin")).toInt();
        m_seats = message.value(QStringLiteral("seats")).toList();
        if (!wasActive && active())
            m_actions.clear();
        emit tableChanged();
    } else if (t == QLatin1String("log")) {
        m_actions = message.value(QStringLiteral("acts")).toList();
        setSynced(true);
        emit reset();
    } else if (t == QLatin1String("act")) {
        // The server sends them in order; a gap means a lost line, so start over.
        if (message.value(QStringLiteral("n")).toInt() != m_actions.size()) {
            drop();
            return;
        }
        const QVariantMap a = message.value(QStringLiteral("a")).toMap();
        m_actions << a;
        emit action(a);
    } else if (t == QLatin1String("players")) {
        m_players = message.value(QStringLiteral("players")).toList();
        emit playersChanged();
    } else if (t == QLatin1String("invited")) {
        if (atTable())
            return;
        m_invitation = {{QStringLiteral("id"), message.value(QStringLiteral("id"))},
                        {QStringLiteral("title"), message.value(QStringLiteral("title"))},
                        {QStringLiteral("from"), message.value(QStringLiteral("from"))}};
        emit invitationChanged();
    } else if (t == QLatin1String("error")) {
        const QString msg = message.value(QStringLiteral("msg")).toString();
        if (msg == QLatin1String("update")) {
            // The server hangs up: trying again would not help.
            disconnectFromServer();
            setError(tr("Update the app to play online"));
        } else if (msg == QLatin1String("wrong password"))
            setError(tr("Wrong password"));
        else if (msg == QLatin1String("no such player"))
            setError(tr("The player has gone"));
        else if (msg == QLatin1String("game full"))
            setError(tr("The table is full"));
        else if (msg == QLatin1String("no such game")) {
            setError(tr("The game is gone"));
            clearTable();
        } else
            setError(msg);
    }
}

void Session::onDisconnected()
{
    m_ping.stop();
    m_watchdog.stop();
    setSynced(false);
    // Already noticed: the socket can report the same loss more than once.
    if (m_status == Offline || m_retry.isActive())
        return;
    setStatus(m_wanted ? Connecting : Offline);
    if (m_wanted) {
        m_retry.start(m_retryMs);
        m_retryMs = qMin(2 * m_retryMs, MaxRetryMs);
    }
}

void Session::drop()
{
    if (m_status != Online)
        setError(tr("The server does not answer"));
    m_socket.abort();
    onDisconnected();
}

void Session::setSynced(bool synced)
{
    if (synced == m_synced)
        return;
    m_synced = synced;
    emit syncedChanged();
}

void Session::setStatus(Status status)
{
    if (status == m_status)
        return;
    m_status = status;
    emit statusChanged();
}

void Session::setError(const QString &error)
{
    if (error == m_error)
        return;
    m_error = error;
    emit errorChanged();
}

void Session::clearTable()
{
    if (m_tableId.isEmpty() && m_seat < 0)
        return;
    m_tableId.clear();
    m_settings.remove(QStringLiteral("online/table"));
    m_title.clear();
    m_started = false;
    m_locked = false;
    m_seat = m_driver = m_admin = -1;
    m_seats.clear();
    m_actions.clear();
    setSynced(false);
    m_invited.clear();
    emit invitedChanged();
    emit tableChanged();
}

void Session::invite(const QString &playerId)
{
    if (!atTable() || m_started || m_invited.contains(playerId))
        return;
    write({{QStringLiteral("t"), QStringLiteral("invite")}, {QStringLiteral("id"), playerId}});
    m_invited << playerId;
    emit invitedChanged();
}

void Session::acceptInvitation()
{
    const QString id = m_invitation.value(QStringLiteral("id")).toString();
    declineInvitation();
    if (!id.isEmpty())
        join(id);
}

void Session::declineInvitation()
{
    if (m_invitation.isEmpty())
        return;
    m_invitation.clear();
    emit invitationChanged();
}
