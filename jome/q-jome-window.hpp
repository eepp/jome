/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_Q_JOME_WINDOW_HPP
#define _JOME_Q_JOME_WINDOW_HPP

#include <QObject>
#include <QEvent>
#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QPixmap>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <boost/optional.hpp>
#include <functional>

#include "emoji-db.hpp"
#include "emoji-images.hpp"
#include "q-emoji-graphics-item.hpp"

namespace jome {

class QSearchBoxEventFilter :
    public QObject
{
    Q_OBJECT

public:
    explicit QSearchBoxEventFilter(QObject *parent);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void upKeyPressed();
    void rightKeyPressed();
    void downKeyPressed();
    void leftKeyPressed();
    void enterKeyPressed();
    void f1KeyPressed();
    void f2KeyPressed();
    void f3KeyPressed();
    void f4KeyPressed();
    void f5KeyPressed();
    void pgUpKeyPressed();
    void pgDownKeyPressed();
    void homeKeyPressed();
    void endKeyPressed();
};

class QJomeWindow :
    public QDialog
{
    Q_OBJECT

public:
    using EmojiChosenFunc = std::function<void (const Emoji&, Emoji::SkinTone)>;

public:
    explicit QJomeWindow(const EmojiDb& emojiDb,
                         const EmojiChosenFunc& emojiChosenFunc);

private:
    void showEvent(QShowEvent *event) override;
    void _setMainStyleSheet();
    void _buildUi();
    void _buildAllEmojisGraphicsScene();
    QListWidget *_createCatListWidget();
    void _showAllEmojis();
    void _setGraphicsSceneStyle(QGraphicsScene& gs);
    void _findEmojis(const std::string& cat, const std::string& needles);
    void _selectEmojiGraphicsItem(const boost::optional<unsigned int>& index);
    QGraphicsPixmapItem *_createSelectedGraphicsItem();
    void _updateInfoLabel(const Emoji *emoji);
    const Emoji *_selectedEmoji();
    void _acceptEmoji(Emoji::SkinTone skinTone);

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
    void _searchBoxUpKeyPressed();
    void _searchBoxRightKeyPressed();
    void _searchBoxDownKeyPressed();
    void _searchBoxLeftKeyPressed();
    void _searchBoxEnterKeyPressed();
    void _searchBoxF1KeyPressed();
    void _searchBoxF2KeyPressed();
    void _searchBoxF3KeyPressed();
    void _searchBoxF4KeyPressed();
    void _searchBoxF5KeyPressed();
    void _searchBoxPgUpKeyPressed();
    void _searchBoxPgDownKeyPressed();
    void _searchBoxHomeKeyPressed();
    void _searchBoxEndKeyPressed();

private:
    const EmojiDb * const _emojiDb;
    const EmojiImages _emojiImages;
    const EmojiChosenFunc _emojiChosenFunc;
    QListWidget *_wCatList = nullptr;
    QLabel *_wInfoLabel = nullptr;
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
