/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_CTL_Q_CTL_CLIENT_HPP
#define _JOME_CTL_Q_CTL_CLIENT_HPP

#include <QObject>
#include <QLocalSocket>

namespace jome {

class QCtlClient :
    public QObject
{
    Q_OBJECT

public:
    enum class Command {
        PICK,
        QUIT,
    };

public:
    explicit QCtlClient(QObject *parent, const std::string& name);
    void ctl(Command cmd);

signals:
    void error();
    void serverReplied(const std::string& str);

private slots:
    void _socketConnected();
    void _socketReadyRead();
    void _socketError(QLocalSocket::LocalSocketError socketError);

private:
    void _connectToServer();
    void _sendString(const std::string& str);

private:
    QLocalSocket _socket;
    Command _curCmd;
    std::string _tmpData;
};

} // namespace jome

#endif // _JOME_CTL_Q_CTL_CLIENT_HPP
