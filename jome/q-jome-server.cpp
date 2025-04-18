/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <cassert>

#include "q-jome-server.hpp"

namespace jome {

QJomeServer::QJomeServer(QObject * const parent, const QString& name) :
    QObject {parent}
{
    QObject::connect(&_server, &QLocalServer::newConnection, this, &QJomeServer::_newConnection);
    _server.listen(name);
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

    // receive one byte at a time until the null terminator
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

void QJomeServer::sendToClient(const QString& str)
{
    if (!_socket || _socket->state() != QLocalSocket::ConnectedState) {
        // not connected for some reason
        return;
    }

    const auto utf8Data = str.toUtf8();

    _socket->write(utf8Data.constData(), utf8Data.size());
    _socket->write(QByteArray {1, '\0'}, 1);
}

} // namespace jome
