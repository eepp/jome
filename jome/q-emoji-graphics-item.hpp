/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_Q_EMOJI_GRAPHICS_ITEM_HPP
#define _JOME_Q_EMOJI_GRAPHICS_ITEM_HPP

#include <QPixmap>
#include <QGraphicsPixmapItem>

#include "emoji-db.hpp"
#include "emoji-images.hpp"

namespace jome {

class QEmojiGraphicsItem :
    public QGraphicsPixmapItem
{
public:
    explicit QEmojiGraphicsItem(const Emoji& emoji, const QPixmap& pixmap);

    const Emoji& emoji() const noexcept
    {
        return *_emoji;
    }

private:
    const Emoji * const _emoji;
};

} // namespace jome

#endif // _JOME_Q_EMOJI_GRAPHICS_ITEM_HPP
