/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "q-emoji-widget.hpp"

namespace jome {

QEmojiWidget::QEmojiWidget(const Emoji& emoji, const QPixmap& pixmap,
                           QWidget * const parent) :
    QLabel {"", parent},
    _emoji {&emoji}
{
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->resize(32, 32);
    this->setPixmap(pixmap);

    static const char * const styleSheet =
        "QLabel:hover {"
        "  background-color: rgba(0, 0, 0, 0.2);"
        "}";

    this->setStyleSheet(styleSheet);
}

} // namespace jome
