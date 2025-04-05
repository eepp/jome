/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <cassert>

#include "q-jome-server.hpp"

namespace jome {

QJomeServer::QJomeServer(QObject * const parent, const std::string& name) :
    QObject {parent}
{
    QObject::connect(&_server, &QLocalServer::newConnection, this, &QJomeServer::_newConnection);
    _server.listen(QString::fromStdString(name));
}

void QJomeServer::_newConnection()
{
    auto socket = _server.nextPendingConnection();

    if (!socket) {
        return;
    }

    QObject::connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);

    if (_socket) {
        // we should probably be able to handle more than one
        socket->close();
        return;
    }

    _socket = socket;
    QObject::connect(socket, &QLocalSocket::disconnected, this, &QJomeServer::_socketDisconnected);
    QObject::connect(socket, &QLocalSocket::readyRead, this, &QJomeServer::_socketReadyRead);
    _tmpData.clear();
}

void QJomeServer::_socketDisconnected()
{
    _socket = nullptr;
}

void QJomeServer::_socketReadyRead()
{
    assert(_socket);

    while (_socket->bytesAvailable() > 0) {
        char byte;

        {
            const auto count = _socket->read(&byte, 1);

            static_cast<void>(count);
            assert(count == 1);
        }

        if (byte == '\0') {
            if (_tmpData == "pick") {
                emit clientRequested(Command::Pick);
                _tmpData.clear();
            } else if (_tmpData == "quit") {
                emit clientRequested(Command::Quit);
                _tmpData.clear();
            }

            continue;
        }

        _tmpData += byte;
    }
}

void QJomeServer::sendToClient(const std::string& str)
{
    if (!_socket || _socket->state() != QLocalSocket::ConnectedState) {
        return;
    }

    _socket->write(str.c_str(), str.size() + 1);
}

} // namespace jome
