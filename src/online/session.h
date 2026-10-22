#pragma once

#include <QObject>
#include <QSettings>
#include <QTcpSocket>
#include <QTimer>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

// A player's connection to cards-server: the lobby, the table you sit at and
// the log of moves made there. The server only orders the moves; every app
// replays them through its own rules, so a game controller turns each one into
// a move in its game. Seats here are the table's; a controller turns them
// round so that you sit at its seat 0.
class Session : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString server READ server WRITE setServer NOTIFY serverChanged)
    Q_PROPERTY(int status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(QVariantList games READ games NOTIFY gamesChanged)
    Q_PROPERTY(QVariantList players READ players NOTIFY playersChanged)
    Q_PROPERTY(bool listed READ listed WRITE setListed NOTIFY listedChanged)
    Q_PROPERTY(QStringList invited READ invited NOTIFY invitedChanged)
    Q_PROPERTY(QVariantMap invitation READ invitation NOTIFY invitationChanged)
    Q_PROPERTY(bool atTable READ atTable NOTIFY tableChanged)
    Q_PROPERTY(bool active READ active NOTIFY tableChanged)
    Q_PROPERTY(QString title READ title NOTIFY tableChanged)
    Q_PROPERTY(bool started READ started NOTIFY tableChanged)
    Q_PROPERTY(bool locked READ locked NOTIFY tableChanged)
    Q_PROPERTY(int seat READ seat NOTIFY tableChanged)
    Q_PROPERTY(bool admin READ admin NOTIFY tableChanged)
    Q_PROPERTY(QVariantList seats READ seats NOTIFY tableChanged)
    Q_PROPERTY(int seatCount READ seatCount NOTIFY tableChanged)
    Q_PROPERTY(int minSeats READ minSeats CONSTANT)
    Q_PROPERTY(int maxSeats READ maxSeats CONSTANT)
    Q_PROPERTY(bool synced READ synced NOTIFY syncedChanged)

public:
    enum Status { Offline, Connecting, Online };
    Q_ENUM(Status)

    // `game` names the card game on the server; `seats` is how many play it,
    // or the fewest when a new table may have up to `maxSeats`.
    Session(const QString &game, int seats, int maxSeats = 0, QObject *parent = nullptr);
    ~Session() override;

    QString name() const { return m_name; }
    void setName(const QString &name);
    // host:port
    QString server() const { return m_server; }
    void setServer(const QString &server);
    int status() const { return m_status; }
    QString error() const { return m_error; }
    // The tables of this game: id, title, players, locked, started and mine.
    QVariantList games() const { return m_games; }
    // The others of this game in the lobby who can be invited: id and name.
    QVariantList players() const { return m_players; }
    // Shown to others in their list of players.
    bool listed() const { return m_listed; }
    void setListed(bool listed);
    // The ids of the players invited to your table.
    QStringList invited() const { return m_invited; }
    // The latest invitation to a table: id, title and from; empty when none.
    QVariantMap invitation() const { return m_invitation; }

    bool atTable() const { return !m_tableId.isEmpty() && m_seat >= 0; }
    // At a table whose game has begun: the controller plays it.
    bool active() const { return atTable() && m_started; }
    QString title() const { return m_title; }
    bool started() const { return m_started; }
    bool locked() const { return m_locked; }
    int seat() const { return m_seat; }
    // How many sit at your table.
    int seatCount() const { return m_seats.size(); }
    int minSeats() const { return m_minSeats; }
    int maxSeats() const { return m_maxSeats; }
    // May start the game, and give an absent player's seat to a bot.
    bool admin() const { return atTable() && m_admin == m_seat; }
    // Each seat's name, human, online and bot.
    QVariantList seats() const { return m_seats; }

    // Seats turned round so that you sit at 0, as a controller numbers its
    // players: from the table's seats, -1 for no such seat, and back. Left
    // as they are when no online game is on.
    int toLocal(int seat) const;
    Q_INVOKABLE int toTable(int player) const;

    // Plays the bots, deals and moves the game on: one app at the table does.
    bool driver() const { return atTable() && m_driver == m_seat; }
    bool isBot(int seat) const;
    // A player who has lost the connection, with no bot playing meanwhile.
    Q_INVOKABLE bool away(int seat) const;
    // A player's name, or a stand-in for a bot.
    Q_INVOKABLE QString playerName(int seat) const;
    // The log has come since sitting down or the last lost connection.
    bool synced() const { return active() && m_synced; }
    // The moves so far, in order.
    const QVariantList &actions() const { return m_actions; }
    // Adds a move to the log; it comes back through `action` like everyone's.
    void send(const QVariantMap &action);

    Q_INVOKABLE void connectToServer();
    Q_INVOKABLE void disconnectFromServer();
    // A table of `seats`, or of the fewest when 0.
    Q_INVOKABLE void create(const QString &title, const QString &password, int seats = 0);
    Q_INVOKABLE void join(const QString &id, const QString &password = QString());
    Q_INVOKABLE void leave();
    Q_INVOKABLE void start();
    Q_INVOKABLE void setBot(int seat, bool on);
    // Asks a player from `players` to your table, before the start.
    Q_INVOKABLE void invite(const QString &playerId);
    Q_INVOKABLE void acceptInvitation();
    Q_INVOKABLE void declineInvitation();

signals:
    void nameChanged();
    void serverChanged();
    void statusChanged();
    void errorChanged();
    void gamesChanged();
    void playersChanged();
    void listedChanged();
    void invitedChanged();
    void invitationChanged();
    void tableChanged();
    // The whole log came, on joining or coming back: replay `actions()`.
    void reset();
    void syncedChanged();
    // One more move, after those already in `actions()`.
    void action(const QVariantMap &action);

private:
    void write(const QVariantMap &message);
    void receive(const QVariantMap &message);
    void setStatus(Status status);
    void setError(const QString &error);
    void clearTable();
    void onReadyRead();
    void onDisconnected();
    // Gives up on the connection and tries again.
    void drop();
    void setSynced(bool synced);

    const QString m_game;
    const int m_minSeats;
    const int m_maxSeats;
    QSettings m_settings;
    QTcpSocket m_socket;
    QTimer m_ping;
    QTimer m_retry;
    // Gives up on a connection that takes too long to open, or goes quiet:
    // on the move, a dead connection rarely says so.
    QTimer m_watchdog;
    int m_retryMs;
    QString m_name;
    QString m_server;
    QString m_token;
    Status m_status = Offline;
    QString m_error;
    // Whether to stay connected, coming back after the network drops.
    bool m_wanted = false;
    QVariantList m_games;
    QVariantList m_players;
    bool m_listed = true;
    QStringList m_invited;
    QVariantMap m_invitation;

    QString m_tableId;
    QString m_title;
    bool m_started = false;
    bool m_locked = false;
    int m_seat = -1;
    int m_driver = -1;
    int m_admin = -1;
    QVariantList m_seats;
    QVariantList m_actions;
    bool m_synced = false;
};
