/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_Q_JOME_SERVER_HPP
#define _JOME_Q_JOME_SERVER_HPP

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>

namespace jome {

/*
 * A server which listens to `jome-ctl` connections, receives commands,
 * and replies accordingly.
 */
class QJomeServer final :
    public QObject
{
    Q_OBJECT

public:
    /*
     * Input command.
     */
    enum class Command
    {
        // pick and emoji (show the window)
        Pick,

        // terminate the server
        Quit,
    };

public:
    /*
     * Builds a jome server using the socket named `name`.
     */
    explicit QJomeServer(QObject *parent, const QString& name);

    /*
     * Sends the message `str` (null-terminated UTF-8 data) to the
     * connected client.
     */
    void sendToClient(const QString& str);

signals:
    /*
     * A connected client requested the command `cmd`.
     */
    void clientRequested(Command cmd);

private slots:
    void _newConnection();
    void _socketDisconnected();
    void _socketReadyRead();

private:
    QLocalServer _server;
    QLocalSocket *_socket = nullptr;
    QString _tmpData;
};

} // namespace jome

#endif // _JOME_Q_JOME_SERVER_HPP
