/*
 * Copyright (C) 2019-2025 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_Q_EMOJI_GRID_WIDGET_HPP
#define _JOME_Q_EMOJI_GRID_WIDGET_HPP

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

/*
 * Emoji grid widget.
 *
 * This is the main widget of jome where the user can select an emoji
 * and navigate the emojis, either all of them by category or the
 * current find results.
 *
 * An emoji grid widget is connected to an emoji database to know what
 * to present. It handles resize events gracefully, ensuring a minimum
 * width of six emojis plus any required padding.
 *
 * Behind the scenes, an emoji grid widget is a Qt graphics view. Each
 * emoji is a graphics item of class `QEmojiGraphicsItem` simply showing
 * a pixmap which is a section of the selected big emoji image (see
 * `EmojiDb::emojisPngPath`). The selection squares are also their own
 * graphics items (`_allEmojisGraphicsSceneSelectedItem`
 * and `_findEmojisGraphicsSceneSelectedItem`).
 *
 * When you build an emoji grid widget, it shows all the emojis by
 * category by default. This is equivalent to calling showAllEmojis(),
 * and showingAllEmojis() returns true. Show find results with a given
 * list of emojis with showFindResults().
 *
 * All the select*() methods are navigation operations.
 *
 * Scroll to a specific category when showing all the emojis
 * with scrollToCat().
 *
 * Your signals of interest are:
 *
 * selectionChanged():
 *     The permanent selection changed to a given emoji.
 *
 * emojiHoverEntered():
 *     The cursor entered a given emoji.
 *
 * emojiHoverLeaved():
 *     The cursor leaved a given emoji.
 *
 * emojiClicked():
 *     The user clicked a given emoji.
 */
class QEmojiGridWidget final :
    public QGraphicsView
{
    Q_OBJECT

    friend class QEmojiGraphicsItem;

public:
    using CatVerticalPositions = std::unordered_map<const EmojiCat *, qreal>;

public:
    explicit QEmojiGridWidget(QWidget *parent, const EmojiDb& emojiDb,
                              bool darkBg, bool noCatLabels,
                              const boost::optional<unsigned int>& selectedEmojiFlashPeriod);

    ~QEmojiGridWidget();
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
    void emojiClicked(const Emoji& emoji, bool withShift);

private:
    void resizeEvent(QResizeEvent *event) override;
    void _selectEmojiGraphicsItem(const boost::optional<unsigned int>& index);
    QGraphicsPixmapItem *_createSelectedGraphicsItem();
    void _setGraphicsSceneStyle(QGraphicsScene& gs);
    void _emojiGraphicsItemHoverEntered(const QEmojiGraphicsItem& item);
    void _emojiGraphicsItemHoverLeaved(const QEmojiGraphicsItem& item);
    void _emojiGraphicsItemClicked(const QEmojiGraphicsItem& item, bool withShift);
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
    // padding used throughout
    static constexpr qreal _gutter = 8.;

private:
    // linked emoji database
    const EmojiDb * const _emojiDb;

    // emoji images
    const EmojiImages _emojiImages;

    // graphics scenes for all emojis and find results
    QGraphicsScene _allEmojisGraphicsScene;
    QGraphicsScene _findEmojisGraphicsScene;

    // vertical positions for each category
    CatVerticalPositions _catVertPositions;

    // current graphics items for all emojis and find results
    std::vector<QEmojiGraphicsItem *> _allEmojiGraphicsItems;
    std::vector<QEmojiGraphicsItem *> _curEmojiGraphicsItems;

    // index of the selected emoji graphics item
    boost::optional<unsigned int> _selectedEmojiGraphicsItemIndex;

    // selection square graphics items for all emojis and find results
    QGraphicsPixmapItem *_allEmojisGraphicsSceneSelectedItem = nullptr;
    QGraphicsPixmapItem *_findEmojisGraphicsSceneSelectedItem = nullptr;

    // timer to make the selection square flash if requested
    QTimer *_selectedItemFlashTimer = nullptr;

    // true to use a dark background
    bool _darkBg;

    // true to hide category labels when showing all emojis
    bool _noCatLabels;
};

} // namespace jome

#endif // _JOME_Q_EMOJI_GRID_WIDGET_HPP
