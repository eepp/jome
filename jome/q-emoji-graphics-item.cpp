/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <QGraphicsSceneMouseEvent>
#include <cstdio>

#include "q-emoji-graphics-item.hpp"
#include "q-emojis-widget.hpp"

namespace jome {

QEmojiGraphicsItem::QEmojiGraphicsItem(const Emoji& emoji, const QPixmap& pixmap,
                                       QEmojisWidget& emojisWidget) :
    QGraphicsPixmapItem {pixmap},
    _emoji {&emoji},
    _emojisWidget {&emojisWidget}
{
    this->setAcceptHoverEvents(true);
    this->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
}

void QEmojiGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent * const event)
{
    if (event->button() == Qt::LeftButton) {
        _emojisWidget->_emojiGraphicsItemClicked(*this);
    }

    QGraphicsPixmapItem::mousePressEvent(event);
}

void QEmojiGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent * const event)
{
    this->setOpacity(.5);
    _emojisWidget->_emojiGraphicsItemHoverEntered(*this);
    QGraphicsPixmapItem::hoverEnterEvent(event);
}

void QEmojiGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * const event)
{
    this->setOpacity(1.);
    _emojisWidget->_emojiGraphicsItemHoverLeaved(*this);
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}

} // namespace jome
