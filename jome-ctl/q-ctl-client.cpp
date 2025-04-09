/*
 * Copyright (C) 2019-2025 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <cassert>

#include "q-ctl-client.hpp"

namespace jome {

QCtlClient::QCtlClient(QObject * const parent, const std::string& name) :
    QObject {parent}
{
    _socket.setServerName(QString::fromStdString(name));
    QObject::connect(&_socket, &QLocalSocket::connected, this, &QCtlClient::_socketConnected);
    QObject::connect(&_socket, &QLocalSocket::readyRead, this, &QCtlClient::_socketReadyRead);
    QObject::connect(&_socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error),
                     this, &QCtlClient::_socketError);
}

void QCtlClient::_connectToServer()
{
    _socket.connectToServer();
}

void QCtlClient::ctl(const Command cmd)
{
    _curCmd = cmd;
    this->_connectToServer();
}

void QCtlClient::_sendString(const std::string& str)
{
    _socket.write(str.c_str(), str.size() + 1);
}

void QCtlClient::_socketConnected()
{
    switch (_curCmd) {
    case Command::Pick:
        this->_sendString("pick");
        break;

    case Command::Quit:
        this->_sendString("quit");
        break;
    }
}

void QCtlClient::_socketReadyRead()
{
    while (_socket.bytesAvailable() > 0) {
        char byte;

        {
            const auto count = _socket.read(&byte, 1);

            static_cast<void>(count);
            assert(count == 1);
        }

        if (byte == '\0') {
            // end of message
            if (_tmpData.empty()) {
                emit this->serverCancelled();
            } else {
                emit this->serverReplied(_tmpData);
            }

            _tmpData.clear();
        } else {
            _tmpData += byte;
        }
    }
}

void QCtlClient::_socketError(const QLocalSocket::LocalSocketError)
{
    emit this->error();
}

} // namespace jome
