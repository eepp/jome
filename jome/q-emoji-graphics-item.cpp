/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <cstdio>

#include "q-emoji-graphics-item.hpp"

namespace jome {

QEmojiGraphicsItem::QEmojiGraphicsItem(const Emoji& emoji,
                                       const QPixmap& pixmap,
                                       const SelectEmojiFunc& pressFunc,
                                       const SelectEmojiFunc& hoverEnterFunc,
                                       const SelectEmojiFunc& hoverLeaveFunc) :
    QGraphicsPixmapItem {pixmap},
    _emoji {&emoji},
    _pressFunc {pressFunc},
    _hoverEnterFunc {hoverEnterFunc},
    _hoverLeaveFunc {hoverLeaveFunc}
{
    this->setAcceptHoverEvents(true);
    this->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
}

void QEmojiGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent * const event)
{
    _pressFunc(*_emoji);
    QGraphicsPixmapItem::mousePressEvent(event);
}

void QEmojiGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent * const event)
{
    this->setOpacity(.5);
    _hoverEnterFunc(*_emoji);
    QGraphicsPixmapItem::hoverEnterEvent(event);
}

void QEmojiGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * const event)
{
    this->setOpacity(1.);
    _hoverLeaveFunc(*_emoji);
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}

} // namespace jome
