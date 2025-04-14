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

namespace jome {

class QEmojiGridWidget;

/*
 * A scene item for a specific emoji.
 *
 * Takes care of changing the opacity when hovering and creating a
 * context menu when right-clicking.
 */
class QEmojiGraphicsItem final :
    public QGraphicsPixmapItem
{
public:
    /*
     * Builds an emoji item for the emoji `emoji` using the image
     * `pixmap` within the emoji grid widget `QEmojisWidget`.
     */
    explicit QEmojiGraphicsItem(const Emoji& emoji, const QPixmap& pixmap,
                                QEmojiGridWidget& emojisWidget);

    /*
     * Corresponding emoji of this item.
     */
    const Emoji& emoji() const noexcept
    {
        return *_emoji;
    }

private:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    const Emoji * const _emoji;
    QEmojiGridWidget * const _emojiGridWidget;
};

} // namespace jome

#endif // _JOME_Q_EMOJI_GRAPHICS_ITEM_HPP
