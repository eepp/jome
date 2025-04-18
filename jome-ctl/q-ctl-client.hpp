/*
 * Copyright (C) 2019-2025 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_CTL_Q_CTL_CLIENT_HPP
#define _JOME_CTL_Q_CTL_CLIENT_HPP

#include <QObject>
#include <QLocalSocket>

namespace jome {

class QCtlClient final :
    public QObject
{
    Q_OBJECT

public:
    enum class Command
    {
        Pick,
        Quit,
    };

public:
    explicit QCtlClient(QObject *parent, const QString& name);
    void ctl(Command cmd);

signals:
    void error();
    void serverReplied(const QString& str);
    void serverCancelled();

private slots:
    void _socketConnected();
    void _socketReadyRead();
    void _socketError(QLocalSocket::LocalSocketError socketError);

private:
    void _connectToServer();
    void _sendString(const QString& str);

private:
    QLocalSocket _socket;
    Command _curCmd;
    std::vector<char> _tmpData;
};

} // namespace jome

#endif // _JOME_CTL_Q_CTL_CLIENT_HPP
