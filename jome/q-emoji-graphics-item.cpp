/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "q-emoji-graphics-item.hpp"

namespace jome {

QEmojiGraphicsItem::QEmojiGraphicsItem(const Emoji& emoji,
                                       const QPixmap& pixmap) :
    QGraphicsPixmapItem {pixmap},
    _emoji {&emoji}
{
}

} // namespace jome
