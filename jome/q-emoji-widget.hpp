/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_Q_EMOJI_WIDGET_HPP
#define _JOME_Q_EMOJI_WIDGET_HPP

#include <QLabel>
#include <QPixmap>

#include "emoji-db.hpp"
#include "emoji-images.hpp"

namespace jome {

class QEmojiWidget :
    public QLabel
{
    Q_OBJECT

public:
    explicit QEmojiWidget(const Emoji& emoji, const QPixmap& pixmap,
                          QWidget *parent = nullptr);

private:
    const Emoji * const _emoji;

};

} // namespace jome

#endif // _JOME_Q_EMOJI_WIDGET_HPP
