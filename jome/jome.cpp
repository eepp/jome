/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QString>
#include <QProcess>
#include <QTimer>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <boost/optional.hpp>

#include "emoji-db.hpp"
#include "emoji-images.hpp"
#include "q-jome-window.hpp"
#include "q-jome-server.hpp"
#include "settings.hpp"

enum class Format
{
    UTF8,
    CODEPOINTS_HEX,
};

struct Params
{
    Format fmt;
    bool noNewline;
    bool noHide;
    bool darkBg;
    boost::optional<std::string> serverName;
    boost::optional<std::string> cmd;
    std::string cpPrefix;
    jome::EmojiDb::EmojiSize emojiSize;
};

static Params parseArgs(QApplication& app)
{
    QCommandLineParser parser;

    parser.setApplicationDescription("An emoji picker desktop application");
    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption formatOpt {"f", "Output format (`utf-8` or `cp`)", "FORMAT", "utf-8"};
    const QCommandLineOption serverNameOpt {"s", "Server name", "NAME"};
    const QCommandLineOption cmdOpt {"c", "External command", "CMD"};
    const QCommandLineOption cpPrefixOpt {"p", "Codepoint prefix", "CPPREFIX"};
    const QCommandLineOption noNlOpt {"n", "Do not output newline"};
    const QCommandLineOption noHideOpt {"q", "Do not quit when accepting"};
    const QCommandLineOption darkBgOpt {"d", "Use dark emoji background"};
    const QCommandLineOption emojiWidthOpt {"w", "Emoji width (16, 24, 32, 40, or 48)", "WIDTH"};

    parser.addOption(formatOpt);
    parser.addOption(serverNameOpt);
    parser.addOption(cmdOpt);
    parser.addOption(cpPrefixOpt);
    parser.addOption(noNlOpt);
    parser.addOption(noHideOpt);
    parser.addOption(darkBgOpt);
    parser.addOption(emojiWidthOpt);
    parser.process(app);

    Params params;

    params.noNewline = parser.isSet(noNlOpt);
    params.noHide = parser.isSet(noHideOpt);
    params.darkBg = parser.isSet(darkBgOpt);

    const auto fmt = parser.value(formatOpt);

    if (fmt == "utf-8") {
        params.fmt = Format::UTF8;
    } else if (fmt == "cp") {
        params.fmt = Format::CODEPOINTS_HEX;
    } else {
        std::cerr << "Command-line error: unknown format `" << fmt.toUtf8().constData() << "`." <<
                     std::endl;
        std::exit(1);
    }

    if (parser.isSet(serverNameOpt)) {
        if (params.noHide) {
            std::cerr << "Command-line error: cannot specify `-s` and `-q` options together." <<
                         std::endl;
            std::exit(1);
        }

        params.serverName = parser.value(serverNameOpt).toUtf8().constData();
    }

    if (parser.isSet(cmdOpt)) {
        params.cmd = parser.value(cmdOpt).toUtf8().constData();
    }

    if (parser.isSet(cpPrefixOpt)) {
        params.cpPrefix = parser.value(cpPrefixOpt).toUtf8().constData();
    }

    params.emojiSize = jome::EmojiDb::EmojiSize::SIZE_32;

    if (parser.isSet(emojiWidthOpt)) {
        const auto val = parser.value(emojiWidthOpt);

        if (val == "16") {
            params.emojiSize = jome::EmojiDb::EmojiSize::SIZE_16;
        } else if (val == "24") {
            params.emojiSize = jome::EmojiDb::EmojiSize::SIZE_24;
        } else if (val == "32") {
            params.emojiSize = jome::EmojiDb::EmojiSize::SIZE_32;
        } else if (val == "40") {
            params.emojiSize = jome::EmojiDb::EmojiSize::SIZE_40;
        } else if (val == "48") {
            params.emojiSize = jome::EmojiDb::EmojiSize::SIZE_48;
        } else {
            std::cerr << "Command-line error: unexpected value for `-w`: `" <<
                         val.toUtf8().constData() << "`." << std::endl;
            std::exit(1);
        }
    }

    return params;
}

static void execCommand(const std::string& cmd, const std::string& arg)
{
    const auto fullCmd = QString::fromStdString(cmd) + " " + QString::fromStdString(arg);

    static_cast<void>(QProcess::execute(fullCmd));
}

static std::string formatEmoji(const jome::Emoji& emoji, const jome::Emoji::SkinTone skinTone,
                               const Format fmt, const std::string& cpPrefix, const bool noNl)
{
    std::string output;

    switch (fmt) {
    case Format::UTF8:
    {
        if (emoji.hasSkinToneSupport()) {
            output = emoji.strWithSkinTone(skinTone);
        } else {
            output = emoji.str();
        }

        break;
    }

    case Format::CODEPOINTS_HEX:
    {
        jome::Emoji::Codepoints codepoints;

        if (emoji.hasSkinToneSupport()) {
            codepoints = emoji.codepointsWithSkinTone(skinTone);
        } else {
            codepoints = emoji.codepoints();
        }

        for (const auto codepoint : codepoints) {
            std::array<char, 32> buf;

            std::sprintf(buf.data(), "%s%x ", cpPrefix.c_str(), codepoint);
            output += buf.data();
        }

        // remove trailing space
        output.resize(output.size() - 1);
        break;
    }
    }

    if (!noNl) {
        output += '\n';
    }

    return output;
}

static void showWindow(jome::QJomeWindow& win, jome::EmojiDb& db)
{
    jome::updateRecentEmojisFromSettings(db);
    QTimer::singleShot(0, &win, &jome::QJomeWindow::emojiDbChanged);
    win.show();
}

int main(int argc, char **argv)
{
    QApplication app {argc, argv};
    std::unique_ptr<jome::QJomeServer> server;

    app.setApplicationDisplayName("jome");
    app.setOrganizationName("jome");
    app.setApplicationName("jome");
    app.setApplicationVersion(JOME_VERSION);

    const auto params = parseArgs(app);
    jome::EmojiDb db {JOME_DATA_DIR, params.emojiSize};
    jome::QJomeWindow win {db, params.darkBg};

    QObject::connect(&win, &jome::QJomeWindow::canceled, [&params, &app, &server]() {
        if (server) {
            // reply to the client at least
            server->sendToClient("");
        }

        if (!server) {
            // TODO: make sure the message is sent before quitting
            QTimer::singleShot(0, [&app]() {
                app.exit(1);
            });
        }
    });
    QObject::connect(&win, &jome::QJomeWindow::emojiChosen,
                     [&](const auto& emoji, const auto skinTone) {
        const auto emojiStr = formatEmoji(emoji, skinTone, params.fmt, params.cpPrefix,
                                          params.noNewline || params.cmd);

        if (server) {
            // send response to client
            server->sendToClient(emojiStr);
        }

        // print result
        std::cout << emojiStr;
        std::cout.flush();

        if (params.cmd) {
            // execute command in 20 ms
            QTimer::singleShot(20, &app, [&params, &server, &app, emojiStr]() {
                execCommand(*params.cmd, emojiStr);

                if (!server && !params.noHide) {
                    // no server: quit after executing the command
                    QTimer::singleShot(0, &app, &QApplication::quit);
                }
            });
        } else {
            if (!server && !params.noHide) {
                // no server: quit now
                QTimer::singleShot(0, &app, &QApplication::quit);
            }
        }

        // always hide when accepting
        if (!params.noHide) {
            win.hide();
        }

        /*
         * Update recent emojis from settings first as it's possible
         * that another jome instance changed them.
         */
        jome::updateRecentEmojisFromSettings(db);

        // add emoji as recent emoji
        db.addRecentEmoji(emoji);

        // DB changed: update settings accordingly
        jome::updateSettings(db);

        if (server || params.noHide) {
            /*
             * Not calling directly because we're potentially within an
             * event handler which is currently using an emoji graphics
             * item, so we cannot delete it.
             */
            QTimer::singleShot(0, &win, &jome::QJomeWindow::emojiDbChanged);
        }
    });

    if (params.serverName) {
        server = std::make_unique<jome::QJomeServer>(nullptr, *params.serverName);
        QObject::connect(server.get(), &jome::QJomeServer::clientRequested,
                         [&app, &server, &win, &db](const jome::QJomeServer::Command cmd) {
            if (cmd == jome::QJomeServer::Command::QUIT) {
                // reply to client, then quit
                server->sendToClient("");

                // TODO: make sure the message is sent before quitting
                QTimer::singleShot(10, &QApplication::quit);
            } else {
                showWindow(win, db);
            }
        });
    }

    if (!server) {
        showWindow(win, db);
    }

    return app.exec();
}
