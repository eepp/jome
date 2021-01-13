/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <QSettings>
#include <vector>

#include "settings.hpp"

namespace jome {

void updateRecentEmojisFromSettings(EmojiDb& db)
{
    std::vector<const Emoji *> recentEmojis;
    QSettings settings;

    const auto recentEmojisVar = settings.value("recent-emojis");

    if (!recentEmojisVar.canConvert<QList<QVariant>>()) {
        return;
    }

    const auto recentEmojisList = recentEmojisVar.toList();

    for (const auto& emojiStrVar : recentEmojisList) {
        if (!emojiStrVar.canConvert<QString>()) {
            continue;
        }

        const auto byteArray = emojiStrVar.toString().toUtf8();
        const auto emojiStr = byteArray.constData();

        recentEmojis.push_back(&db.emojiForStr(emojiStr));
    }

    db.recentEmojis(std::move(recentEmojis));
}

void updateSettings(const EmojiDb& db)
{
    QList<QVariant> emojiList;

    for (const auto emoji : db.recentEmojisCat().emojis()) {
        const auto emojiStr = QString::fromStdString(emoji->str());

        emojiList.append(emojiStr);
    }

    // update settings and write immediately
    QSettings settings;

    settings.setValue("recent-emojis", emojiList);
    settings.sync();
}

} // namespace jome
