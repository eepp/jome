/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include <QString>
#include <iostream>

#include "q-ctl-client.hpp"

struct Params
{
    jome::QCtlClient::Command cmd = jome::QCtlClient::Command::PICK;
    std::string serverName;
};

namespace {

Params parseArgs(QCoreApplication& app)
{
    QCommandLineParser parser;

    parser.setApplicationDescription("Control jome");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("SERVER-NAME", "jome server name", "NAME");
    parser.addPositionalArgument("CMD", "Command (`pick` or `quit`)", "CMD");
    parser.process(app);

    Params params;
    const auto posArgs = parser.positionalArguments();

    if (posArgs.isEmpty()) {
        std::cerr << "Command-line error: missing server name." << std::endl;
        std::exit(1);
    }

    params.serverName = parser.positionalArguments().first().toUtf8().constData();

    if (posArgs.size() >= 2) {
        const auto& cmd = posArgs[1];

        if (cmd == "quit") {
            params.cmd = jome::QCtlClient::Command::QUIT;
        } else if (cmd != "pick") {
            std::cerr << "Command-line error: unknown command `" <<
                         cmd.toUtf8().constData() << "`." << std::endl;
            std::exit(1);
        }
    }

    return params;
}

} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app {argc, argv};

    app.setApplicationName("jome-ctl");
    app.setOrganizationName("jome");
    app.setApplicationVersion(JOME_VERSION);

    const auto params = parseArgs(app);

    jome::QCtlClient client {nullptr, params.serverName};

    QObject::connect(&client, &jome::QCtlClient::serverReplied,
                     [&params, &app](const std::string& str) {
        if (params.cmd == jome::QCtlClient::Command::PICK) {
            std::cout << str;
            std::cout.flush();
        }

        // ignore response for other commands, just quit
        QTimer::singleShot(0, &app, &QCoreApplication::quit);
    });
    QObject::connect(&client, &jome::QCtlClient::error, [&app]() {
        // just quit
        QTimer::singleShot(0, &app, [&app]() {
            app.exit(1);
        });
    });

    client.ctl(params.cmd);
    return app.exec();
}
