/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_JOME_WINDOW_HPP
#define _JOME_JOME_WINDOW_HPP

#include <QDialog>
#include <QListWidget>
#include <QScrollArea>

#include "emoji-db.hpp"
#include "emoji-images.hpp"

namespace jome {

class QJomeWindow :
    public QDialog
{
    Q_OBJECT

public:
    explicit QJomeWindow(const EmojiDb& emojiDb);

private:
    void showEvent(QShowEvent *event);
    void _setMainStyleSheet();
    void _buildUi();
    QListWidget *_createCatListWidget();
    void _showAllEmojis();
    void _findEmojis(const std::string& cat, const std::string& needles);

private slots:
    void _searchTextChanged(const QString& text);
    void _catListItemSelectionChanged();
    void _catListItemClicked(QListWidgetItem *item);

private:
    const EmojiDb * const _emojiDb;
    const EmojiImages _emojiImages;
    QListWidget *_wCatList = nullptr;
    QScrollArea *_wEmojisArea = nullptr;
    QWidget *_wAllEmojisAreaWidget = nullptr;
    std::unordered_map<const EmojiCat *, int> _catVertPositions;
};

} // namespace jome

#endif // _JOME_JOME_WINDOW_HPP
