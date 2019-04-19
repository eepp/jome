/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <QApplication>
#include <QLabel>
#include <cstdio>

#include "emoji-db.hpp"
#include "emoji-images.hpp"

int main(int argc, char **argv)
{
    QApplication app {argc, argv};

    jome::EmojiDb db {JOME_DATA_DIR};
    jome::EmojiImages images {db};
    auto& emoji = db.emojiForStr("🦊");

    QLabel lbl {""};
    lbl.resize(32, 32);
    lbl.setPixmap(images.pixmapForEmoji(emoji));
    lbl.show();
    return app.exec();
}
