/*
 * Copyright (C) 2019-2025 Philippe Proulx <eepp.ca>
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
#include <QTimer>
#include <boost/optional.hpp>
#include <cmath>

#include "emoji-db.hpp"
#include "emoji-images.hpp"
#include "q-emoji-graphics-item.hpp"

namespace jome {

class QEmojisWidget final :
    public QGraphicsView
{
    Q_OBJECT

    friend class QEmojiGraphicsItem;

public:
    using CatVerticalPositions = std::unordered_map<const EmojiCat *, qreal>;

public:
    explicit QEmojisWidget(QWidget *parent, const EmojiDb& emojiDb,
                           bool darkBg,
                           const boost::optional<unsigned int>& selectedEmojiFlashPeriod);

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
    void resizeEvent(QResizeEvent *event) override;
    void _selectEmojiGraphicsItem(const boost::optional<unsigned int>& index);
    QGraphicsPixmapItem *_createSelectedGraphicsItem();
    void _setGraphicsSceneStyle(QGraphicsScene& gs);
    void _emojiGraphicsItemHoverEntered(const QEmojiGraphicsItem& item);
    void _emojiGraphicsItemHoverLeaved(const QEmojiGraphicsItem& item);
    void _emojiGraphicsItemClicked(const QEmojiGraphicsItem& item);
    void _addRoundedRectToScene(QGraphicsScene& gs, qreal y, qreal height);

private slots:
    void _selectedItemFlashTimerTimeout();

private:
    qreal _rowFirstEmojiX(const QGraphicsScene& gs) const
    {
        const auto availWidth = gs.width() - _gutter * 4;
        const auto rowEmojiCount = std::floor((availWidth + _gutter) / (_emojiDb->emojiSizeInt() + _gutter));
        const auto emojisTotalWidth = rowEmojiCount * _emojiDb->emojiSizeInt() +
                                      (rowEmojiCount - 1) * _gutter;

        return std::floor((availWidth - emojisTotalWidth) / 2.) + _gutter * 2;
    }

    template <typename ContainerT>
    void _addEmojisToGraphicsScene(const ContainerT& emojis,
                                   std::vector<QEmojiGraphicsItem *>& emojiGraphicsItems,
                                   QGraphicsScene& gs, qreal& y)
    {
        qreal col = 0.;
        const auto rowFirstEmojiX = this->_rowFirstEmojiX(gs);
        const auto emojiWidthAndMargin = _emojiDb->emojiSizeInt() + _gutter;

        for (auto& emoji : emojis) {
            auto emojiGraphicsItem = new QEmojiGraphicsItem {
                *emoji, _emojiImages.pixmapForEmoji(*emoji), *this
            };

            emojiGraphicsItems.push_back(emojiGraphicsItem);
            emojiGraphicsItem->setPos(col * emojiWidthAndMargin + rowFirstEmojiX, y);
            gs.addItem(emojiGraphicsItem);
            col += 1;

            if ((col + 1.) * emojiWidthAndMargin + rowFirstEmojiX >= gs.width()) {
                col = 0.;
                y += emojiWidthAndMargin;
            }
        }

        if (col != 0.) {
            y += emojiWidthAndMargin;
        }

        y -= _gutter;
    }

private:
    static constexpr qreal _gutter = 8.;

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
    QTimer _selectedItemFlashTimer;
    bool _darkBg;
};

} // namespace jome

#endif // _JOME_Q_EMOJIS_WIDGET_HPP
