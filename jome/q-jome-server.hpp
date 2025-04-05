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

class QJomeServer final :
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
    explicit QJomeServer(QObject *parent, const std::string& name);
    void sendToClient(const std::string& str);

signals:
    void clientRequested(Command cmd);

private slots:
    void _newConnection();
    void _socketDisconnected();
    void _socketReadyRead();

private:
    QLocalServer _server;
    QLocalSocket *_socket = nullptr;
    std::string _tmpData;
};

} // namespace jome

#endif // _JOME_Q_JOME_SERVER_HPP
