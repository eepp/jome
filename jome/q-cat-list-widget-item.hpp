/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_Q_CAT_LIST_WIDGET_ITEM_HPP
#define _JOME_Q_CAT_LIST_WIDGET_ITEM_HPP

#include <QListWidget>

#include "emoji-db.hpp"

namespace jome {

/*
 * An entry within the list of categories.
 *
 * Contains a category pointer.
 */
class QCatListWidgetItem final :
    public QListWidgetItem
{
public:
    explicit QCatListWidgetItem(const EmojiCat& cat);

    const EmojiCat& cat() const noexcept
    {
        return *_cat;
    }

private:
    const EmojiCat *_cat;
};

} // namespace jome

#endif // _JOME_Q_CAT_LIST_WIDGET_ITEM_HPP
