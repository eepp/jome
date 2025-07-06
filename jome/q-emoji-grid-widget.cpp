/*
 * Copyright (C) 2019-2025 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QScrollArea>
#include <QScrollBar>
#include <QListWidget>
#include <QLabel>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <boost/algorithm/string.hpp>

#include "q-emoji-grid-widget.hpp"
#include "utils.hpp"

namespace jome {

QEmojiGridWidget::QEmojiGridWidget(QWidget * const parent, const EmojiDb& emojiDb, const bool darkBg,
                                   const bool noCatLabels,
                                   const boost::optional<unsigned int>& selectedEmojiFlashPeriod) :
    QGraphicsView {parent},
    _emojiDb {&emojiDb},
    _emojiImages {emojiDb},
    _darkBg {darkBg},
    _noCatLabels {noCatLabels}
{
    _allEmojisGraphicsSceneSelectedItem = this->_createSelectedGraphicsItem();
    _findEmojisGraphicsSceneSelectedItem = this->_createSelectedGraphicsItem();
    this->_setGraphicsSceneStyle(_allEmojisGraphicsScene);
    this->_setGraphicsSceneStyle(_findEmojisGraphicsScene);

    if (selectedEmojiFlashPeriod) {
        _selectedItemFlashTimer = new QTimer {this};
        QObject::connect(&*_selectedItemFlashTimer, &QTimer::timeout,
                         this, &QEmojiGridWidget::_selectedItemFlashTimerTimeout);
        _selectedItemFlashTimer->setInterval(*selectedEmojiFlashPeriod / 2);
        _selectedItemFlashTimer->start();
    }

    this->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setFocusPolicy(Qt::NoFocus);

    // margins, 6 emojis, and scrollbar
    this->setMinimumWidth(static_cast<int>(_gutter * 4 +
                                           (_emojiDb->emojiSizeInt() + _gutter) * 6 +
                                           _gutter + 1));
}

QEmojiGridWidget::~QEmojiGridWidget()
{
    /*
     * Those "selected" graphics scene items could be out of the
     * scene currently, therefore owned by this.
     */
    if (!_findEmojisGraphicsSceneSelectedItem->scene()) {
        delete _findEmojisGraphicsSceneSelectedItem;
    }

    if (!_allEmojisGraphicsSceneSelectedItem->scene()) {
        delete _allEmojisGraphicsSceneSelectedItem;
    }
}

void QEmojiGridWidget::_selectedItemFlashTimerTimeout()
{
    if (_allEmojisGraphicsSceneSelectedItem) {
        _allEmojisGraphicsSceneSelectedItem->setVisible(!_allEmojisGraphicsSceneSelectedItem->isVisible());
    }

    if (_findEmojisGraphicsSceneSelectedItem) {
        _findEmojisGraphicsSceneSelectedItem->setVisible(!_findEmojisGraphicsSceneSelectedItem->isVisible());
    }
}

void QEmojiGridWidget::_setGraphicsSceneStyle(QGraphicsScene& gs)
{
    gs.setBackgroundBrush(QColor {_darkBg ? "#404040" : "#d0d0d0"});
}

QGraphicsPixmapItem *QEmojiGridWidget::_createSelectedGraphicsItem()
{
    auto graphicsItem = new QGraphicsPixmapItem {
        QPixmap::fromImage(QImage {
            qFmtFormat("{}/sel{}-{}.png", JOME_DATA_DIR, _darkBg ? "-dark" : "",
                       _emojiDb->emojiSizeInt())
        })
    };

    graphicsItem->hide();
    graphicsItem->setEnabled(false);
    graphicsItem->setZValue(-5.);
    return graphicsItem;
}

void QEmojiGridWidget::_addRoundedRectToScene(QGraphicsScene& gs, const qreal y, const qreal height)
{
    QPainterPath path;

    path.addRoundedRect(QRectF(0., 0., gs.width() - _gutter * 2, height), _gutter, _gutter);

    const auto item = gs.addPath(path);

    item->setPos(_gutter, y);
    item->setPen(Qt::NoPen);
    item->setBrush(QColor {_darkBg ? "#202020" : "#f8f8f8"});
    item->setZValue(-2000.);
}

void QEmojiGridWidget::rebuild()
{
    if (_allEmojisGraphicsSceneSelectedItem->scene()) {
        _allEmojisGraphicsScene.removeItem(_allEmojisGraphicsSceneSelectedItem);
    }

    _allEmojisGraphicsScene.clear();
    _allEmojisGraphicsScene.addItem(_allEmojisGraphicsSceneSelectedItem);
    _allEmojiGraphicsItems.clear();
    _catVertPositions.clear();

    // scene width: width of this widget minus scrollbar width
    _allEmojisGraphicsScene.setSceneRect(0., 0., static_cast<qreal>(this->width()) - _gutter, 0.);

    const auto rowFirstEmojiX = this->_rowFirstEmojiX(_allEmojisGraphicsScene);
    qreal y = _gutter;

    for (const auto& cat : _emojiDb->cats()) {
        _catVertPositions[cat.get()] = y;

        const auto rectBeginY = y;

        y += _gutter;

        if (!_noCatLabels) {
            {
                auto item = _allEmojisGraphicsScene.addText(cat->name(),
                                                            QFont {"Hack, DejaVu Sans Mono, monospace",
                                                                   10, QFont::Bold});

                item->setDefaultTextColor(QColor {_darkBg ? "#f8f8f8" : "#202020"});
                item->setPos(rowFirstEmojiX, y);
            }

            y += 32.;
        }

        this->_addEmojisToGraphicsScene(cat->emojis(), _allEmojiGraphicsItems,
                                        _allEmojisGraphicsScene, y);
        y += _gutter;
        this->_addRoundedRectToScene(_allEmojisGraphicsScene, rectBeginY, y - rectBeginY);
        y += _gutter;
    }

    _allEmojisGraphicsScene.setSceneRect(0., 0., static_cast<qreal>(this->width()) - _gutter, y);
}

void QEmojiGridWidget::showAllEmojis()
{
    _curEmojiGraphicsItems = _allEmojiGraphicsItems;
    this->setScene(&_allEmojisGraphicsScene);
    this->_selectEmojiGraphicsItem(0);
}

void QEmojiGridWidget::showFindResults(const std::vector<const Emoji *>& results)
{
    if (_findEmojisGraphicsSceneSelectedItem->scene()) {
        _findEmojisGraphicsScene.removeItem(_findEmojisGraphicsSceneSelectedItem);
    }

    _findEmojisGraphicsScene.clear();
    _findEmojisGraphicsScene.addItem(_findEmojisGraphicsSceneSelectedItem);
    _curEmojiGraphicsItems.clear();

    // scene width: width of this widget minus scrollbar width
    _findEmojisGraphicsScene.setSceneRect(0., 0., static_cast<qreal>(this->width()) - _gutter, 0.);

    qreal y = 0.;

    if (!results.empty()) {
        y = _gutter;

        const auto rectBeginY = y;

        y += _gutter;
        this->_addEmojisToGraphicsScene(results, _curEmojiGraphicsItems,
                                        _findEmojisGraphicsScene, y);
        y += _gutter;
        this->_addRoundedRectToScene(_findEmojisGraphicsScene, rectBeginY, y - rectBeginY);
        y += _gutter;
    }

    _findEmojisGraphicsScene.setSceneRect(0., 0., static_cast<qreal>(this->width()) - 8., y);
    this->setScene(&_findEmojisGraphicsScene);

    if (results.empty()) {
        this->_selectEmojiGraphicsItem(boost::none);
    } else {
        this->_selectEmojiGraphicsItem(0);
    }
}

void QEmojiGridWidget::_emojiGraphicsItemHoverEntered(const QEmojiGraphicsItem& item)
{
    emit this->emojiHoverEntered(item.emoji());
}

void QEmojiGridWidget::_emojiGraphicsItemHoverLeaved(const QEmojiGraphicsItem& item)
{
    emit this->emojiHoverLeaved(item.emoji());
}

void QEmojiGridWidget::_emojiGraphicsItemClicked(const QEmojiGraphicsItem& item,
                                                 const bool withShift)
{
    emit this->emojiClicked(item.emoji(), withShift);
}

void QEmojiGridWidget::_selectEmojiGraphicsItem(const boost::optional<unsigned int>& index)
{
    const auto selectedItem = call([this] {
        if (this->showingAllEmojis()) {
            return _allEmojisGraphicsSceneSelectedItem;
        } else {
            return _findEmojisGraphicsSceneSelectedItem;
        }
    });

    assert(selectedItem);
    assert(selectedItem->scene());
    _selectedEmojiGraphicsItemIndex = index;

    if (!index) {
        if (_selectedItemFlashTimer) {
            _selectedItemFlashTimer->stop();
        }

        selectedItem->hide();
        emit this->selectionChanged(nullptr);
        return;
    }

    assert(*index < _curEmojiGraphicsItems.size());

    const auto& emojiGraphicsItem = *_curEmojiGraphicsItems[*index];

    selectedItem->show();

    if (_selectedItemFlashTimer) {
        _selectedItemFlashTimer->start();
    }

    selectedItem->setPos(emojiGraphicsItem.pos().x() - 4.,
                         emojiGraphicsItem.pos().y() - 4.);

    if (*index == 0) {
        this->verticalScrollBar()->setValue(0);
    } else {
        const auto candY = selectedItem->pos().y() + 16. -
                           static_cast<qreal>(this->height()) / 2.;
        const auto y = std::max(0., candY);

        this->verticalScrollBar()->setValue(static_cast<int>(y));
    }

    emit this->selectionChanged(&emojiGraphicsItem.emoji());
}

void QEmojiGridWidget::scrollToCat(const EmojiCat& cat)
{
    if (_catVertPositions.empty()) {
        return;
    }

    this->verticalScrollBar()->setValue(static_cast<int>(std::max(0.,
                                                                  _catVertPositions[&cat] - 8)));
}

void QEmojiGridWidget::selectNext(const unsigned int count)
{
    if (!_selectedEmojiGraphicsItemIndex) {
        return;
    }

    for (auto i = 0U; i < count; ++i) {
        if (*_selectedEmojiGraphicsItemIndex + 1 == _curEmojiGraphicsItems.size()) {
            return;
        }

        this->_selectEmojiGraphicsItem(*_selectedEmojiGraphicsItemIndex + 1);
    }
}

void QEmojiGridWidget::selectPrevious(const unsigned int count)
{
    if (!_selectedEmojiGraphicsItemIndex) {
        return;
    }

    for (auto i = 0U; i < count; ++i) {
        if (*_selectedEmojiGraphicsItemIndex == 0) {
            return;
        }

        this->_selectEmojiGraphicsItem(*_selectedEmojiGraphicsItemIndex - 1);
    }
}

void QEmojiGridWidget::selectPreviousRow(const unsigned int count)
{
    if (!_selectedEmojiGraphicsItemIndex) {
        return;
    }

    const auto& selectedEmojiGraphicsItem = *_curEmojiGraphicsItems[*_selectedEmojiGraphicsItemIndex];
    const auto curX = selectedEmojiGraphicsItem.pos().x();
    auto index = *_selectedEmojiGraphicsItemIndex;

    for (auto i = 0U; i < count; ++i) {
        for (auto eI = static_cast<int>(index) - 1; eI >= 0; --eI) {
            const auto& emojiGraphicsItem = *_curEmojiGraphicsItems[eI];

            if (emojiGraphicsItem.pos().x() == curX) {
                index = static_cast<unsigned int>(eI);
                break;
            }
        }
    }

    this->_selectEmojiGraphicsItem(index);
}

void QEmojiGridWidget::selectNextRow(const unsigned int count)
{
    if (!_selectedEmojiGraphicsItemIndex) {
        return;
    }

    const auto& selectedEmojiGraphicsItem = *_curEmojiGraphicsItems[*_selectedEmojiGraphicsItemIndex];
    const auto curX = selectedEmojiGraphicsItem.pos().x();
    auto index = *_selectedEmojiGraphicsItemIndex;

    for (auto i = 0U; i < count; ++i) {
        for (auto eI = index + 1; eI < _curEmojiGraphicsItems.size(); ++eI) {
            const auto& emojiGraphicsItem = *_curEmojiGraphicsItems[eI];

            if (emojiGraphicsItem.pos().x() == curX) {
                index = eI;
                break;
            }
        }
    }

    this->_selectEmojiGraphicsItem(index);
}

void QEmojiGridWidget::selectFirst()
{
    if (_curEmojiGraphicsItems.empty()) {
        return;
    }

    this->_selectEmojiGraphicsItem(0);
}

void QEmojiGridWidget::selectLast()
{
    if (_curEmojiGraphicsItems.empty()) {
        return;
    }

    this->_selectEmojiGraphicsItem(_curEmojiGraphicsItems.size() - 1);
}

bool QEmojiGridWidget::showingAllEmojis()
{
    return this->scene() == &_allEmojisGraphicsScene;
}

void QEmojiGridWidget::resizeEvent(QResizeEvent * const event)
{
    QGraphicsView::resizeEvent(event);

    // save current index
    const auto selectedItemIndex = _selectedEmojiGraphicsItemIndex;

    // rebuild the "all emojis" view
    this->rebuild();

    if (this->showingAllEmojis()) {
        this->showAllEmojis();
    } else {
        std::vector<const Emoji *> results;

        results.reserve(_curEmojiGraphicsItems.size());

        for (const auto item : _curEmojiGraphicsItems) {
            results.push_back(&item->emoji());
        }

        this->showFindResults(results);
    }

    this->_selectEmojiGraphicsItem(selectedItemIndex);
}

} // namespace jome
