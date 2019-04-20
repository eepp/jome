/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_Q_JOME_WINDOW_HPP
#define _JOME_Q_JOME_WINDOW_HPP

#include <QDialog>
#include <QListWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QPixmap>

#include "emoji-db.hpp"
#include "emoji-images.hpp"
#include "q-emoji-widget.hpp"

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
    void _createSelPixmal();
    void _showAllEmojis();
    void _findEmojis(const std::string& cat, const std::string& needles);

private:
    template <typename ContainerT>
    QGridLayout *_createEmojiGridLayout(const ContainerT& emojis)
    {
        auto grid = new QGridLayout;

        grid->setSpacing(8);
        grid->setMargin(0);

        auto col = 0U;
        auto row = 0U;
        const auto availWidth = static_cast<unsigned int>(_wEmojisArea->viewport()->width() - 16);

        for (const auto& emoji : emojis) {
            auto wEmoji = new QEmojiWidget {*emoji,
                                            _emojiImages.pixmapForEmoji(*emoji),
                                            *_selPixmap};

            grid->addWidget(wEmoji, row, col);
            col += 1;

            if ((col + 1) * (32 + 8) >= availWidth) {
                col = 0;
                row += 1;
            }
        }

        return grid;
    }

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
    std::unique_ptr<QPixmap> _selPixmap;
};

} // namespace jome

#endif // _JOME_Q_JOME_WINDOW_HPP
