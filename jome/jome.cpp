/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QString>
#include <iostream>
#include <cstdio>
#include <cstdlib>

#include "emoji-db.hpp"
#include "emoji-images.hpp"
#include "q-jome-window.hpp"

enum class Format {
    UTF8,
    CODEPOINTS_HEX,
    CODEPOINTS_HEX_U_PREFIX,
};

struct Params
{
    Format fmt;
    bool noNewline;
};

static Params parseArgs(QApplication& app, int argc, char **argv)
{
    QCommandLineParser parser;

    parser.setApplicationDescription("An emoji picker desktop application");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption formatOpt {"f", "Output format", "FORMAT", "utf-8"};
    QCommandLineOption noNlOpt {"n", "Do not output newline"};

    parser.addOption(formatOpt);
    parser.addOption(noNlOpt);
    parser.process(app);

    Params params;

    params.noNewline = parser.isSet(noNlOpt);

    const auto fmt = parser.value(formatOpt);

    if (fmt == "utf-8") {
        params.fmt = Format::UTF8;
    } else if (fmt == "cp") {
        params.fmt = Format::CODEPOINTS_HEX;
    } else if (fmt == "ucp") {
        params.fmt = Format::CODEPOINTS_HEX_U_PREFIX;
    } else {
        std::cerr << "Command-line error: unknown format `" <<
                     fmt.toUtf8().constData() << "`." << std::endl;
        std::exit(1);
    }

    return params;
}

int main(int argc, char **argv)
{
    QApplication app {argc, argv};

    app.setApplicationDisplayName("jome");
    app.setApplicationName("jome");
    app.setApplicationVersion(JOME_VERSION);

    const auto params = parseArgs(app, argc, argv);
    const jome::EmojiDb db {JOME_DATA_DIR};
    jome::QJomeWindow win {db, [&params](const auto& emoji) {
        switch (params.fmt) {
        case Format::UTF8:
            std::printf("%s", emoji.str().c_str());
            break;

        case Format::CODEPOINTS_HEX:
        case Format::CODEPOINTS_HEX_U_PREFIX:
            for (const auto codepoint : emoji.codepoints()) {
                if (params.fmt == Format::CODEPOINTS_HEX_U_PREFIX) {
                    std::printf("U+%X ", codepoint);
                } else {
                    std::printf("%x ", codepoint);
                }
            }

            break;

        default:
            std::abort();
        }

        if (!params.noNewline) {
            std::printf("\n");
        }

        std::fflush(stdout);
    }};

    win.show();
    return app.exec();
}
