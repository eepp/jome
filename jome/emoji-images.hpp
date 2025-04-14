/*
 * Copyright (C) 2019-2025 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_EMOJI_IMAGES_HPP
#define _JOME_EMOJI_IMAGES_HPP

#include <memory>
#include <unordered_map>
#include <QPixmap>

#include "emoji-db.hpp"

namespace jome {

/*
 * All the emoji images.
 *
 * An `EmojiImages` instance holds a map of emoji to
 * corresponding `QPixmap`.
 */
class EmojiImages final
{
public:
    /*
     * Builds all the emoji images from the database `db`.
     */
    explicit EmojiImages(const EmojiDb& db);

    /*
     * Returns the image of the emoji `emoji`.
     */
    const QPixmap& pixmapForEmoji(const Emoji& emoji) const
    {
        return *_emojiPixmaps.at(&emoji);
    }

private:
    /*
     * Fills `_emojiPixmaps` from the database `db`.
     */
    void _createPixmaps(const EmojiDb& db);

private:
    std::unordered_map<const Emoji *, std::unique_ptr<QPixmap>> _emojiPixmaps;
};

} // namespace jome

#endif // _JOME_EMOJI_IMAGES_HPP
