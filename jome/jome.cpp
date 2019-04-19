/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <QApplication>
#include <cstdio>

#include "emoji-db.hpp"

int main(int argc, char **argv)
{
    jome::EmojiDb db {JOME_DATA_DIR};

    for (const auto& emoji : db.emojisForKeyword("water")) {
        printf("%s %s\n", emoji->str().c_str(), emoji->name().c_str());
    }
}
