/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "q-emoji-widget.hpp"

namespace jome {

QEmojiWidget::QEmojiWidget(const Emoji& emoji, const QPixmap& pixmap,
                           const QPixmap& selPixmap, QWidget * const parent) :
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
    _wSelLabel = new QLabel {"", this};
    _wSelLabel->resize(32, 32);
    _wSelLabel->setPixmap(selPixmap);
    _wSelLabel->hide();
}

void QEmojiWidget::select(const bool select)
{
    if (select) {
        _wSelLabel->show();
    } else {
        _wSelLabel->hide();
    }
}

} // namespace jome
