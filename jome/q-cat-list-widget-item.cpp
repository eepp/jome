/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "q-cat-list-widget-item.hpp"

namespace jome {

QCatListWidgetItem::QCatListWidgetItem(const EmojiCat& cat) :
    _cat {&cat}
{
    this->setText(QString::fromStdString(cat.name()));
}

} // namespace jome
