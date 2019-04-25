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
#include "q-emojis-widget.hpp"

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
    explicit QJomeWindow(const EmojiDb& emojiDb);

signals:
    void emojiChosen(const Emoji& emoji, Emoji::SkinTone skinTone);
    void canceled();

private:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void _setMainStyleSheet();
    void _buildUi();
    QListWidget *_createCatListWidget();
    void _updateInfoLabel(const Emoji *emoji);
    void _findEmojis(const std::string& cat, const std::string& needles);
    void _acceptSelectedEmoji(Emoji::SkinTone skinTone);
    void _acceptEmoji(const Emoji& emoji, Emoji::SkinTone skinTone);

private slots:
    void reject() override;
    void accept() override;
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
    void _emojiSelectionChanged(const Emoji *emoji);
    void _emojiClicked(const Emoji& emoji);
    void _emojiHoverEntered(const Emoji& emoji);
    void _emojiHoverLeaved(const Emoji& emoji);

private:
    const EmojiDb * const _emojiDb;
    QEmojisWidget *_wEmojis = nullptr;
    QListWidget *_wCatList = nullptr;
    QLabel *_wInfoLabel = nullptr;
    QLineEdit *_wSearchBox = nullptr;
    bool _emojisWidgetBuilt = false;
    const Emoji *_selectedEmoji = nullptr;
};

} // namespace jome

#endif // _JOME_Q_JOME_WINDOW_HPP
