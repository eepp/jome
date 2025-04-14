/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <QGraphicsSceneMouseEvent>
#include <QMenu>

#include "q-emoji-graphics-item.hpp"
#include "q-emoji-grid-widget.hpp"
#include "emojipedia.hpp"

namespace jome {

QEmojiGraphicsItem::QEmojiGraphicsItem(const Emoji& emoji, const QPixmap& pixmap,
                                       QEmojiGridWidget& emojiGridWidget) :
    QGraphicsPixmapItem {pixmap},
    _emoji {&emoji},
    _emojiGridWidget {&emojiGridWidget}
{
    this->setAcceptHoverEvents(true);
    this->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
}

void QEmojiGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent * const event)
{
    if (event->button() == Qt::LeftButton) {
        _emojiGridWidget->_emojiGraphicsItemClicked(*this, event->modifiers() & Qt::ShiftModifier);
    }

    QGraphicsPixmapItem::mousePressEvent(event);
}

void QEmojiGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent * const event)
{
    this->setOpacity(.5);
    _emojiGridWidget->_emojiGraphicsItemHoverEntered(*this);
    QGraphicsPixmapItem::hoverEnterEvent(event);
}

void QEmojiGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * const event)
{
    this->setOpacity(1.);
    _emojiGridWidget->_emojiGraphicsItemHoverLeaved(*this);
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}

void QEmojiGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent * const event)
{
    QMenu menu;
    const auto requestEmojiInfoAction = menu.addAction("Go to Emojipedia page");
    const auto selAction = menu.exec(event->screenPos());

    if (selAction == requestEmojiInfoAction) {
        gotoEmojipediaPage(*_emoji);
    }
}

} // namespace jome
