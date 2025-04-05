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
#include <qobject.h>

#include "q-emojis-widget.hpp"
#include "utils.hpp"

namespace jome {

QEmojisWidget::QEmojisWidget(QWidget * const parent, const EmojiDb& emojiDb, const bool darkBg,
                             const boost::optional<unsigned int>& selectedEmojiFlashPeriod) :
    QGraphicsView {parent},
    _emojiDb {&emojiDb},
    _emojiImages {emojiDb},
    _darkBg {darkBg}
{
    _allEmojisGraphicsSceneSelectedItem = this->_createSelectedGraphicsItem();
    _findEmojisGraphicsSceneSelectedItem = this->_createSelectedGraphicsItem();
    this->_setGraphicsSceneStyle(_allEmojisGraphicsScene);
    this->_setGraphicsSceneStyle(_findEmojisGraphicsScene);

    if (selectedEmojiFlashPeriod) {
        QObject::connect(&_selectedItemFlashTimer, &QTimer::timeout,
                         this, &QEmojisWidget::_selectedItemFlashTimerTimeout);
        _selectedItemFlashTimer.start(*selectedEmojiFlashPeriod / 2);
    }

    this->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // margins, 6 emojis, and scrollbar
    this->setMinimumWidth(8 + (_emojiDb->emojiSizeInt() + 8) * 7 + 8 + 1);
}

QEmojisWidget::~QEmojisWidget()
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

void QEmojisWidget::_selectedItemFlashTimerTimeout()
{
    if (_allEmojisGraphicsSceneSelectedItem) {
        _allEmojisGraphicsSceneSelectedItem->setVisible(!_allEmojisGraphicsSceneSelectedItem->isVisible());
    }

    if (_findEmojisGraphicsSceneSelectedItem) {
        _findEmojisGraphicsSceneSelectedItem->setVisible(!_findEmojisGraphicsSceneSelectedItem->isVisible());
    }
}

void QEmojisWidget::_setGraphicsSceneStyle(QGraphicsScene& gs)
{
    gs.setBackgroundBrush(QColor {_darkBg ? "#202020" : "#f8f8f8"});
}

QGraphicsPixmapItem *QEmojisWidget::_createSelectedGraphicsItem()
{
    auto graphicsItem = new QGraphicsPixmapItem {
        QPixmap::fromImage(QImage {
            qFmtFormat("{}/sel-{}.png", JOME_DATA_DIR, _emojiDb->emojiSizeInt())
        })
    };

    graphicsItem->hide();
    graphicsItem->setEnabled(false);
    graphicsItem->setZValue(1000.);
    return graphicsItem;
}

void QEmojisWidget::rebuild()
{
    if (_allEmojisGraphicsSceneSelectedItem->scene()) {
        _allEmojisGraphicsScene.removeItem(_allEmojisGraphicsSceneSelectedItem);
    }

    _allEmojisGraphicsScene.clear();
    _allEmojisGraphicsScene.addItem(_allEmojisGraphicsSceneSelectedItem);
    _allEmojiGraphicsItems.clear();
    _catVertPositions.clear();
    _allEmojisGraphicsScene.setSceneRect(0., 0., static_cast<qreal>(this->width()) - 8., 0.);

    const auto rowFirstEmojiX = this->_rowFirstEmojiX(_allEmojisGraphicsScene);
    qreal y = 8.;

    for (const auto& cat : _emojiDb->cats()) {
        auto item = _allEmojisGraphicsScene.addText(QString::fromStdString(cat->name()),
                                                    QFont {"Hack, DejaVu Sans Mono, monospace",
                                                           10, QFont::Bold});

        item->setDefaultTextColor(QColor {_darkBg ? "#f8f8f8" : "#202020"});
        item->setPos(rowFirstEmojiX, y);
        _catVertPositions[cat.get()] = y;
        y += 24.;
        this->_addEmojisToGraphicsScene(cat->emojis(), _allEmojiGraphicsItems,
                                        _allEmojisGraphicsScene, y);
        y += 8.;
    }

    y -= 8.;
    _allEmojisGraphicsScene.setSceneRect(0., 0., static_cast<qreal>(this->width()) - 8., y);
}

void QEmojisWidget::showAllEmojis()
{
    _curEmojiGraphicsItems = _allEmojiGraphicsItems;
    this->setScene(&_allEmojisGraphicsScene);
    this->_selectEmojiGraphicsItem(0);
}

void QEmojisWidget::showFindResults(const std::vector<const Emoji *>& results)
{
    if (_findEmojisGraphicsSceneSelectedItem->scene()) {
        _findEmojisGraphicsScene.removeItem(_findEmojisGraphicsSceneSelectedItem);
    }

    _findEmojisGraphicsScene.clear();
    _findEmojisGraphicsScene.addItem(_findEmojisGraphicsSceneSelectedItem);
    _curEmojiGraphicsItems.clear();
    _findEmojisGraphicsScene.setSceneRect(0., 0., static_cast<qreal>(this->width()) - 8., 0.);

    qreal y = 0.;

    if (!results.empty()) {
        y = 8.;
        this->_addEmojisToGraphicsScene(results, _curEmojiGraphicsItems,
                                        _findEmojisGraphicsScene, y);
    }

    _findEmojisGraphicsScene.setSceneRect(0., 0., static_cast<qreal>(this->width()) - 8., y);
    this->setScene(&_findEmojisGraphicsScene);

    if (results.empty()) {
        this->_selectEmojiGraphicsItem(boost::none);
    } else {
        this->_selectEmojiGraphicsItem(0);
    }
}

void QEmojisWidget::_emojiGraphicsItemHoverEntered(const QEmojiGraphicsItem& item)
{
    emit this->emojiHoverEntered(item.emoji());
}

void QEmojisWidget::_emojiGraphicsItemHoverLeaved(const QEmojiGraphicsItem& item)
{
    emit this->emojiHoverLeaved(item.emoji());
}

void QEmojisWidget::_emojiGraphicsItemClicked(const QEmojiGraphicsItem& item)
{
    emit this->emojiClicked(item.emoji());
}

void QEmojisWidget::_selectEmojiGraphicsItem(const boost::optional<unsigned int>& index)
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
        selectedItem->hide();
        emit this->selectionChanged(nullptr);
        return;
    }

    assert(*index < _curEmojiGraphicsItems.size());

    const auto& emojiGraphicsItem = *_curEmojiGraphicsItems[*index];

    selectedItem->show();
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

void QEmojisWidget::scrollToCat(const EmojiCat& cat)
{
    if (_catVertPositions.empty()) {
        return;
    }

    this->verticalScrollBar()->setValue(static_cast<int>(std::max(0.,
                                                                  _catVertPositions[&cat] - 8)));
}

void QEmojisWidget::selectNext(const unsigned int count)
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

void QEmojisWidget::selectPrevious(const unsigned int count)
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

void QEmojisWidget::selectPreviousRow(const unsigned int count)
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

void QEmojisWidget::selectNextRow(const unsigned int count)
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

void QEmojisWidget::selectFirst()
{
    if (_curEmojiGraphicsItems.empty()) {
        return;
    }

    this->_selectEmojiGraphicsItem(0);
}

void QEmojisWidget::selectLast()
{
    if (_curEmojiGraphicsItems.empty()) {
        return;
    }

    this->_selectEmojiGraphicsItem(_curEmojiGraphicsItems.size() - 1);
}

bool QEmojisWidget::showingAllEmojis()
{
    return this->scene() == &_allEmojisGraphicsScene;
}

void QEmojisWidget::resizeEvent(QResizeEvent * const event)
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
