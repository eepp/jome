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
#include <functional>

#include "emoji-db.hpp"
#include "emoji-images.hpp"

namespace jome {

class QEmojiGraphicsItem :
    public QGraphicsPixmapItem
{
public:
    using SelectEmojiFunc = std::function<void (const Emoji& emoji)>;

public:
    explicit QEmojiGraphicsItem(const Emoji& emoji, const QPixmap& pixmap,
                                const SelectEmojiFunc& pressFunc,
                                const SelectEmojiFunc& hoverEnterFunc,
                                const SelectEmojiFunc& hoverLeaveFunc);

    const Emoji& emoji() const noexcept
    {
        return *_emoji;
    }

private:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    const Emoji * const _emoji;
    const SelectEmojiFunc _pressFunc;
    const SelectEmojiFunc _hoverEnterFunc;
    const SelectEmojiFunc _hoverLeaveFunc;
};

} // namespace jome

#endif // _JOME_Q_EMOJI_GRAPHICS_ITEM_HPP
