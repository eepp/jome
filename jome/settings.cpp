/*
 * Copyright (C) 2019-2025 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <QSettings>
#include <vector>

#include "settings.hpp"
#include "utils.hpp"

namespace jome {

void updateRecentEmojisFromSettings(EmojiDb& db)
{
    QSettings settings;
    const auto recentEmojisVar = settings.value("recent-emojis");

    if (!recentEmojisVar.canConvert<QList<QVariant>>()) {
        return;
    }

    const auto recentEmojisList = recentEmojisVar.toList();
    std::vector<const Emoji *> recentEmojis;

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
    const auto emojiList = call([&db] {
        QList<QVariant> emojiList;

        for (const auto emoji : db.recentEmojisCat().emojis()) {
            const auto emojiStr = QString::fromStdString(emoji->str());

            emojiList.append(emojiStr);
        }

        return emojiList;
    });

    // update settings and write immediately
    QSettings settings;

    settings.setValue("recent-emojis", emojiList);
    settings.sync();
}

} // namespace jome
