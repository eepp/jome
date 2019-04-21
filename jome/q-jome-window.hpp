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
#include <QGraphicsScene>
#include <QGraphicsView>
#include <boost/optional.hpp>

#include "emoji-db.hpp"
#include "emoji-images.hpp"
#include "q-emoji-graphics-item.hpp"

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
    void _buildAllEmojisGraphicsScene();
    QListWidget *_createCatListWidget();
    void _showAllEmojis();
    void _setGraphicsSceneStyle(QGraphicsScene& gs);
    void _findEmojis(const std::string& cat, const std::string& needles);
    void _selectEmojiGraphicsItem(const boost::optional<unsigned int>& index);
    QGraphicsPixmapItem *_createSelectedGraphicsItem();

private:
    template <typename ContainerT>
    void _addEmojisToGraphicsScene(const ContainerT& emojis,
                                   std::vector<QEmojiGraphicsItem *>& emojiGraphicsItems,
                                   QGraphicsScene& gs,
                                   qreal& y)
    {
        qreal col = 0.;
        const auto availWidth = gs.width();
        constexpr auto emojiWidthAndMargin = 32. + 8.;

        for (const auto& emoji : emojis) {
            auto emojiGraphicsItem = new QEmojiGraphicsItem {
                *emoji, _emojiImages.pixmapForEmoji(*emoji)
            };

            emojiGraphicsItems.push_back(emojiGraphicsItem);
            emojiGraphicsItem->setPos(col * emojiWidthAndMargin + 8., y);
            gs.addItem(emojiGraphicsItem);
            col += 1;

            if ((col + 1.) * emojiWidthAndMargin + 8. >= availWidth) {
                col = 0.;
                y += emojiWidthAndMargin;
            }
        }

        if (col != 0.) {
            y += emojiWidthAndMargin;
        }
    }

private slots:
    void _searchTextChanged(const QString& text);
    void _catListItemSelectionChanged();
    void _catListItemClicked(QListWidgetItem *item);

private:
    const EmojiDb * const _emojiDb;
    const EmojiImages _emojiImages;
    QListWidget *_wCatList = nullptr;
    bool _allEmojisGraphicsSceneBuilt = false;
    QGraphicsScene _allEmojisGraphicsScene;
    QGraphicsScene _findEmojisGraphicsScene;
    QGraphicsView *_wEmojisGraphicsView = nullptr;
    std::unordered_map<const EmojiCat *, qreal> _catVertPositions;
    std::vector<QEmojiGraphicsItem *> _curEmojiGraphicsItems;
    std::vector<QEmojiGraphicsItem *> _allEmojiGraphicsItems;
    boost::optional<unsigned int> _selectedEmojiGraphicsItemIndex;
    QGraphicsPixmapItem *_allEmojisGraphicsSceneSelectedItem = nullptr;
    QGraphicsPixmapItem *_findEmojisGraphicsSceneSelectedItem = nullptr;
};

} // namespace jome

#endif // _JOME_Q_JOME_WINDOW_HPP
