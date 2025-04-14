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
#include <fmt/format.h>

#include "emoji-db.hpp"
#include "q-jome-window.hpp"
#include "q-jome-server.hpp"
#include "settings.hpp"
#include "utils.hpp"

/*
 * Output format.
 */
enum class Format
{
    Utf8,
    CodepointsHex,
};

/*
 * Command-line parameters in no particular order.
 */
struct Params final
{
    Format fmt;
    bool noNewline;
    bool noHide;
    bool darkBg;
    bool copyToClipboard;
    boost::optional<QString> serverName;
    boost::optional<QString> cmd;
    QString cpPrefix;
    jome::EmojiDb::EmojiSize emojiSize;
    boost::optional<unsigned int> selectedEmojiFlashPeriod;
    unsigned int maxRecentEmojis;
    bool removeVs16;
    bool noCatList;
    bool noCatLabels;
    bool noRecentCat;
    bool noKwList;
    boost::optional<jome::Emoji::SkinTone> defSkinTone;
    bool incRecentInFindResults;
};

namespace {

Params parseArgs(QApplication& app)
{
    QCommandLineParser parser;

    parser.setApplicationDescription("An emoji picker desktop application");
    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption formatOpt {"f", "Set output format to <FORMAT> (`utf-8` or `cp`)", "FORMAT", "utf-8"};
    const QCommandLineOption cpPrefixOpt {"p", "Set codepoint prefix to <CPPREFIX>.", "CPPREFIX"};
    const QCommandLineOption noNlOpt {"n", "Do not output newline."};
    const QCommandLineOption removeVs16Opt {"V", "Do not output VS-16 codepoints."};
    const QCommandLineOption defSkinToneOpt {"t", "Set default skin tone to <TONE> (`L`, `ML`, `M`, `MD`, or `D`).", "TONE"};
    const QCommandLineOption incRecentInFindResultsOpt {"r", "Include recently accepted emojis in find results."};
    const QCommandLineOption cmdOpt {"c", "Execute external command <CMD> with accepted emoji.", "CMD"};
    const QCommandLineOption copyToClipboardOpt {"b", "Copy the accepted emoji to the clipboard."};
    const QCommandLineOption noHideOpt {"q", "Do not quit when accepting."};
    const QCommandLineOption serverNameOpt {"s", "Set server name to <NAME>.", "NAME"};
    const QCommandLineOption darkBgOpt {"d", "Use dark emoji background."};
    const QCommandLineOption noCatListOpt {"C", "Hide category list."};
    const QCommandLineOption noCatLabelsOpt {"L", "Hide category labels."};
    const QCommandLineOption noRecentCatOpt {"R", "Hide \"Recent\" category."};
    const QCommandLineOption noKwListOpt {"k", "Hide keyword list."};
    const QCommandLineOption emojiWidthOpt {"w", "Set emoji width to <WIDTH> px (16, 24, 32, 40, or 48).", "WIDTH"};
    const QCommandLineOption selectedEmojiFlashPeriodOpt {"P", "Set selected emoji flashing period to <PERIOD> ms.", "PERIOD"};
    const QCommandLineOption maxRecentEmojisOpt {"H", "Set maximum number of recently accepted emojis to <COUNT>.", "COUNT"};

    parser.addOption(formatOpt);
    parser.addOption(cpPrefixOpt);
    parser.addOption(noNlOpt);
    parser.addOption(removeVs16Opt);
    parser.addOption(defSkinToneOpt);
    parser.addOption(incRecentInFindResultsOpt);
    parser.addOption(cmdOpt);
    parser.addOption(copyToClipboardOpt);
    parser.addOption(noHideOpt);
    parser.addOption(serverNameOpt);
    parser.addOption(darkBgOpt);
    parser.addOption(noCatListOpt);
    parser.addOption(noCatLabelsOpt);
    parser.addOption(noRecentCatOpt);
    parser.addOption(noKwListOpt);
    parser.addOption(emojiWidthOpt);
    parser.addOption(selectedEmojiFlashPeriodOpt);
    parser.addOption(maxRecentEmojisOpt);
    parser.process(app);

    Params params;

    params.noNewline = parser.isSet(noNlOpt);
    params.noHide = parser.isSet(noHideOpt);
    params.darkBg = parser.isSet(darkBgOpt);
    params.copyToClipboard = parser.isSet(copyToClipboardOpt);
    params.removeVs16 = parser.isSet(removeVs16Opt);
    params.noCatList = parser.isSet(noCatListOpt);
    params.noCatLabels = parser.isSet(noCatLabelsOpt);
    params.noRecentCat = parser.isSet(noRecentCatOpt);
    params.noKwList = parser.isSet(noKwListOpt);
    params.incRecentInFindResults = parser.isSet(incRecentInFindResultsOpt);

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

    if (parser.isSet(selectedEmojiFlashPeriodOpt)) {
        bool ok;
        const auto strVal = parser.value(selectedEmojiFlashPeriodOpt);
        const auto val = strVal.toUInt(&ok);

        if (!ok || val < 32) {
            std::cerr << "Command-line error: unexpected value for `-P`: `" <<
                         strVal.toUtf8().constData() << "`.\n";
            std::exit(1);
        }

        params.selectedEmojiFlashPeriod = val;
    }

    params.maxRecentEmojis = 30;

    if (parser.isSet(maxRecentEmojisOpt)) {
        bool ok;
        const auto strVal = parser.value(maxRecentEmojisOpt);
        const auto val = strVal.toUInt(&ok);

        if (!ok || val < 1) {
            std::cerr << "Command-line error: unexpected value for `-H`: `" <<
                         strVal.toUtf8().constData() << "`.\n";
            std::exit(1);
        }

        params.maxRecentEmojis = val;
    }

    if (parser.isSet(defSkinToneOpt)) {
        const auto val = parser.value(defSkinToneOpt).toUpper();

        if (val == "L") {
            params.defSkinTone = jome::Emoji::SkinTone::Light;
        } else if (val == "ML") {
            params.defSkinTone = jome::Emoji::SkinTone::MediumLight;
        } else if (val == "M") {
            params.defSkinTone = jome::Emoji::SkinTone::Medium;
        } else if (val == "MD") {
            params.defSkinTone = jome::Emoji::SkinTone::MediumDark;
        } else if (val == "D") {
            params.defSkinTone = jome::Emoji::SkinTone::Dark;
        } else {
            std::cerr << "Command-line error: unexpected value for `-t`: `" <<
                         parser.value(defSkinToneOpt).toUtf8().constData() << "`.\n";
            std::exit(1);
        }
    }

    return params;
}

/*
 * Executes the command `cmd` with the arguments `arg` (shell context).
 */
void execCommand(const QString& cmd, const QString& arg)
{
    static_cast<void>(QProcess::execute(cmd + ' ' + arg));
}

/*
 * Formats the emoji `emoji` with the format `fmt` and returns
 * the string.
 *
 * Adds a skin tone modifier depending on `skinTone` and `defSkinTone`.
 *
 * If `fmt` is `Format::CodepointsHex`, prepends `cpPrefix` to each
 * hexadecimal codepoint.
 *
 * Removes VS-16 codepoints if `removeVs16` is true.
 *
 * Adds a newline if `noNl` is false.
 */
QString formatEmoji(const jome::Emoji& emoji,
                    const boost::optional<jome::Emoji::SkinTone>& skinTone,
                    const boost::optional<jome::Emoji::SkinTone>& defSkinTone,
                    const Format fmt,
                    const QString& cpPrefix, const bool noNl, const bool removeVs16)
{
    QString output;
    const auto realSkinTone = skinTone ? skinTone : defSkinTone;

    switch (fmt) {
    case Format::Utf8:
    {
        if (realSkinTone && emoji.hasSkinToneSupport()) {
            output = emoji.str(*realSkinTone, !removeVs16);
        } else {
            output = emoji.str(boost::none, !removeVs16);
        }

        break;
    }

    case Format::CodepointsHex:
    {
        const auto codepoints = jome::call([realSkinTone, &emoji, &removeVs16] {
            if (realSkinTone && emoji.hasSkinToneSupport()) {
                return emoji.codepoints(*realSkinTone, !removeVs16);
            } else {
                return emoji.codepoints(boost::none, !removeVs16);
            }
        });

        for (const auto codepoint : codepoints) {
            output += jome::qFmtFormat("{}{:x} ", cpPrefix.toStdString(), codepoint);
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

/*
 * Shows the jome window working with the database `db`.
 *
 * Also updates the "Recent" category, if any, from the settings.
 */
void showWindow(jome::QJomeWindow& win, jome::EmojiDb& db)
{
    jome::updateRecentEmojisFromSettings(db);
    QTimer::singleShot(0, &win, &jome::QJomeWindow::emojiDbChanged);
    win.show();
}

} // namespace

int main(int argc, char ** const argv)
{
    // create Qt app
    QApplication app {argc, argv};

    app.setApplicationDisplayName("jome");
    app.setOrganizationName("jome");
    app.setApplicationName("jome");
    app.setApplicationVersion(JOME_VERSION);

    // parse command-line parameters
    const auto params = parseArgs(app);

    // create emoji database
    jome::EmojiDb db {
        JOME_DATA_DIR, params.emojiSize, params.maxRecentEmojis, params.noRecentCat,
        params.incRecentInFindResults
    };

    // create window (not visible yet)
    jome::QJomeWindow win {db, params.darkBg, params.noCatList, params.noCatLabels,
                           params.noKwList, params.selectedEmojiFlashPeriod};

    // possible server
    std::unique_ptr<jome::QJomeServer> server;

    // `QJomeWindow::cancelled` signal
    QObject::connect(&win, &jome::QJomeWindow::cancelled, [&app, &server, &db]() {
        if (server) {
            // reply to the client at least
            server->sendToClient("");
        }

        if (!server) {
            // TODO: make sure the message is sent before quitting
            QTimer::singleShot(0, [&app, &db]() {
                jome::updateSettings(db);
                app.exit(1);
            });
        }
    });

    // `QJomeWindow::emojiChosen` signal
    QObject::connect(&win, &jome::QJomeWindow::emojiChosen,
                     [&](const auto& emoji, const auto& skinTone, const bool removeVs16) {
        // format emoji
        const auto emojiStr = formatEmoji(emoji, skinTone, params.defSkinTone, params.fmt,
                                          params.cpPrefix, params.noNewline || params.cmd,
                                          removeVs16 || params.removeVs16);

        if (server) {
            // send formatted emoji to connected client
            server->sendToClient(emojiStr);
        }

        // always print the formatted emoji
        std::cout << emojiStr.toStdString();
        std::cout.flush();

        if (params.copyToClipboard) {
            QGuiApplication::clipboard()->setText(emojiStr);
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

        // always hide when accepting, except with `-q`
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
        // create server
        server = std::make_unique<jome::QJomeServer>(nullptr, *params.serverName);

        // connect `QJomeServer::clientRequested` signal
        QObject::connect(server.get(), &jome::QJomeServer::clientRequested,
                         [&server, &win, &db](const jome::QJomeServer::Command cmd) {
            if (cmd == jome::QJomeServer::Command::Quit) {
                // reply to client, then quit
                server->sendToClient("");

                // TODO: make sure the message is sent before quitting
                QTimer::singleShot(10, &QApplication::quit);
            } else {
                assert(cmd == jome::QJomeServer::Command::Pick);
                showWindow(win, db);
            }
        });
    }

    if (!server) {
        // direct mode: time to show the window
        showWindow(win, db);
    }

    // start app
    return app.exec();
}
