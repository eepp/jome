/*
 * Copyright (C) 2019-2025 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <QApplication>
#include <QClipboard>
#include <QCommandLineParser>
#include <QString>
#include <QProcess>
#include <QTimer>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <boost/optional.hpp>

#include "emoji-db.hpp"
#include "q-jome-window.hpp"
#include "q-jome-server.hpp"
#include "settings.hpp"
#include "utils.hpp"

enum class Format
{
    Utf8,
    CodepointsHex,
};

struct Params final
{
    Format fmt;
    bool noNewline;
    bool noHide;
    bool darkBg;
    bool copyToClipboard;
    boost::optional<std::string> serverName;
    boost::optional<std::string> cmd;
    std::string cpPrefix;
    jome::EmojiDb::EmojiSize emojiSize;
    boost::optional<unsigned int> selectedEmojiFlashPeriod;
};

namespace {

Params parseArgs(QApplication& app)
{
    QCommandLineParser parser;

    parser.setApplicationDescription("An emoji picker desktop application");
    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption formatOpt {"f", "Set output format to <FORMAT> (`utf-8` or `cp`)", "FORMAT", "utf-8"};
    const QCommandLineOption serverNameOpt {"s", "Set server name to <NAME>.", "NAME"};
    const QCommandLineOption cmdOpt {"c", "Execute external command <CMD> with accepted emoji.", "CMD"};
    const QCommandLineOption copyToClipboardOpt {"b", "Copy the accepted emoji to the clipboard."};
    const QCommandLineOption cpPrefixOpt {"p", "Set codepoint prefix to <CPPREFIX>.", "CPPREFIX"};
    const QCommandLineOption noNlOpt {"n", "Do not output newline."};
    const QCommandLineOption noHideOpt {"q", "Do not quit when accepting."};
    const QCommandLineOption darkBgOpt {"d", "Use dark emoji background."};
    const QCommandLineOption emojiWidthOpt {"w", "Set emoji width to <WIDTH> px (16, 24, 32, 40, or 48).", "WIDTH"};
    const QCommandLineOption selectedEmojiFlashPeriod {"P", "Set selected emoji flashing period to <PERIOD> ms.", "PERIOD"};

    parser.addOption(formatOpt);
    parser.addOption(serverNameOpt);
    parser.addOption(cmdOpt);
    parser.addOption(copyToClipboardOpt);
    parser.addOption(cpPrefixOpt);
    parser.addOption(noNlOpt);
    parser.addOption(noHideOpt);
    parser.addOption(darkBgOpt);
    parser.addOption(emojiWidthOpt);
    parser.addOption(selectedEmojiFlashPeriod);
    parser.process(app);

    Params params;

    params.noNewline = parser.isSet(noNlOpt);
    params.noHide = parser.isSet(noHideOpt);
    params.darkBg = parser.isSet(darkBgOpt);
    params.copyToClipboard = parser.isSet(copyToClipboardOpt);

    {
        const auto fmt = parser.value(formatOpt);

        if (fmt == "utf-8") {
            params.fmt = Format::Utf8;
        } else if (fmt == "cp") {
            params.fmt = Format::CodepointsHex;
        } else {
            std::cerr << "Command-line error: unknown format `" << fmt.toUtf8().constData() << "`.\n";
            std::exit(1);
        }
    }

    if (parser.isSet(serverNameOpt)) {
        if (params.noHide) {
            std::cerr << "Command-line error: cannot specify `-s` and `-q` options together.\n";
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

    params.emojiSize = jome::EmojiDb::EmojiSize::Size32;

    if (parser.isSet(emojiWidthOpt)) {
        const auto val = parser.value(emojiWidthOpt);

        if (val == "16") {
            params.emojiSize = jome::EmojiDb::EmojiSize::Size16;
        } else if (val == "24") {
            params.emojiSize = jome::EmojiDb::EmojiSize::Size24;
        } else if (val == "32") {
            params.emojiSize = jome::EmojiDb::EmojiSize::Size32;
        } else if (val == "40") {
            params.emojiSize = jome::EmojiDb::EmojiSize::Size40;
        } else if (val == "48") {
            params.emojiSize = jome::EmojiDb::EmojiSize::Size48;
        } else {
            std::cerr << "Command-line error: unexpected value for `-w`: `" <<
                         val.toUtf8().constData() << "`.\n";
            std::exit(1);
        }
    }

    if (parser.isSet(selectedEmojiFlashPeriod)) {
        bool ok;
        const auto strVal = parser.value(selectedEmojiFlashPeriod);
        const auto val = strVal.toUInt(&ok);

        if (!ok || val < 32) {
            std::cerr << "Command-line error: unexpected value for `-P`: `" <<
                         strVal.toUtf8().constData() << "`.\n";
            std::exit(1);
        }

        params.selectedEmojiFlashPeriod = val;
    }

    return params;
}

void execCommand(const std::string& cmd, const std::string& arg)
{
    const auto fullCmd = QString::fromStdString(cmd) + " " + QString::fromStdString(arg);

    static_cast<void>(QProcess::execute(fullCmd));
}

std::string formatEmoji(const jome::Emoji& emoji,
                        const boost::optional<jome::Emoji::SkinTone>& skinTone, const Format fmt,
                        const std::string& cpPrefix, const bool noNl)
{
    std::string output;

    switch (fmt) {
    case Format::Utf8:
    {
        if (skinTone && emoji.hasSkinToneSupport()) {
            output = emoji.strWithSkinTone(*skinTone);
        } else {
            output = emoji.str();
        }

        break;
    }

    case Format::CodepointsHex:
    {
        const auto codepoints = jome::call([skinTone, &emoji] {
            if (skinTone && emoji.hasSkinToneSupport()) {
                return emoji.codepointsWithSkinTone(*skinTone);
            } else {
                return emoji.codepoints();
            }
        });

        for (const auto codepoint : codepoints) {
            output += cpPrefix;
            output += QString::number(codepoint, 16).toStdString();
            output += ' ';
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

void showWindow(jome::QJomeWindow& win, jome::EmojiDb& db)
{
    jome::updateRecentEmojisFromSettings(db);
    QTimer::singleShot(0, &win, &jome::QJomeWindow::emojiDbChanged);
    win.show();
}

} // namespace

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
    jome::QJomeWindow win {db, params.darkBg, params.selectedEmojiFlashPeriod};

    QObject::connect(&win, &jome::QJomeWindow::canceled, [&app, &server]() {
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
                     [&](const auto& emoji, const auto& skinTone) {
        const auto emojiStr = formatEmoji(emoji, skinTone, params.fmt, params.cpPrefix,
                                          params.noNewline || params.cmd);

        if (server) {
            // send response to client
            server->sendToClient(emojiStr);
        }

        // print result
        std::cout << emojiStr;
        std::cout.flush();

        if (params.copyToClipboard) {
            QGuiApplication::clipboard()->setText(QString::fromStdString(emojiStr));
        }

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
                         [&server, &win, &db](const jome::QJomeServer::Command cmd) {
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
