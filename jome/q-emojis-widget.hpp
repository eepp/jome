/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_Q_EMOJIS_WIDGET_HPP
#define _JOME_Q_EMOJIS_WIDGET_HPP

#include <QObject>
#include <QEvent>
#include <QPixmap>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <boost/optional.hpp>
#include <functional>

#include "emoji-db.hpp"
#include "emoji-images.hpp"
#include "q-emoji-graphics-item.hpp"

namespace jome {

class QEmojisWidget :
    public QGraphicsView
{
    Q_OBJECT
    friend class QEmojiGraphicsItem;

public:
    using CatVerticalPositions = std::unordered_map<const EmojiCat *, qreal>;

public:
    explicit QEmojisWidget(QWidget *parent, const EmojiDb& emojiDb);
    ~QEmojisWidget();
    void rebuild();
    void showAllEmojis();
    void showFindResults(const std::vector<const Emoji *>& results);
    void selectNext(unsigned int count = 1);
    void selectPrevious(unsigned int count = 1);
    void selectPreviousRow(unsigned int count = 1);
    void selectNextRow(unsigned int count = 1);
    void selectFirst();
    void selectLast();
    void scrollToCat(const EmojiCat& cat);
    bool showingAllEmojis();

signals:
    void selectionChanged(const Emoji *emoji);
    void emojiHoverEntered(const Emoji& emoji);
    void emojiHoverLeaved(const Emoji& emoji);
    void emojiClicked(const Emoji& emoji);

private:
    void _selectEmojiGraphicsItem(const boost::optional<unsigned int>& index);
    QGraphicsPixmapItem *_createSelectedGraphicsItem();
    void _setGraphicsSceneStyle(QGraphicsScene& gs);
    void _emojiGraphicsItemHoverEntered(const QEmojiGraphicsItem& item);
    void _emojiGraphicsItemHoverLeaved(const QEmojiGraphicsItem& item);
    void _emojiGraphicsItemClicked(const QEmojiGraphicsItem& item);

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
            namespace ph = std::placeholders;

            auto emojiGraphicsItem = new QEmojiGraphicsItem {
                *emoji, _emojiImages.pixmapForEmoji(*emoji), *this
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

private:
    const EmojiDb * const _emojiDb;
    const EmojiImages _emojiImages;
    QGraphicsScene _allEmojisGraphicsScene;
    QGraphicsScene _findEmojisGraphicsScene;
    CatVerticalPositions _catVertPositions;
    std::vector<QEmojiGraphicsItem *> _curEmojiGraphicsItems;
    std::vector<QEmojiGraphicsItem *> _allEmojiGraphicsItems;
    boost::optional<unsigned int> _selectedEmojiGraphicsItemIndex;
    QGraphicsPixmapItem *_allEmojisGraphicsSceneSelectedItem = nullptr;
    QGraphicsPixmapItem *_findEmojisGraphicsSceneSelectedItem = nullptr;
};

} // namespace jome

#endif // _JOME_Q_EMOJIS_WIDGET_HPP
