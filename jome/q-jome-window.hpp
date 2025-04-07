/*
 * Copyright (C) 2019-2025 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_Q_JOME_WINDOW_HPP
#define _JOME_Q_JOME_WINDOW_HPP

#include <QObject>
#include <QEvent>
#include <QMainWindow>
#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QPixmap>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <boost/optional.hpp>

#include "emoji-db.hpp"
#include "q-emojis-widget.hpp"

namespace jome {

class QSearchBoxEventFilter final :
    public QObject
{
    Q_OBJECT

public:
    explicit QSearchBoxEventFilter(QObject *parent);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void upKeyPressed();
    void rightKeyPressed(bool withCtrl);
    void downKeyPressed();
    void leftKeyPressed(bool withCtrl);
    void enterKeyPressed(bool withShift);
    void f1KeyPressed(bool withShift);
    void f2KeyPressed(bool withShift);
    void f3KeyPressed(bool withShift);
    void f4KeyPressed(bool withShift);
    void f5KeyPressed(bool withShift);
    void f12KeyPressed();
    void pgUpKeyPressed();
    void pgDownKeyPressed();
    void homeKeyPressed();
    void endKeyPressed();
    void escapeKeyPressed();
};

class QJomeWindow final :
    public QMainWindow
{
    Q_OBJECT

public:
    explicit QJomeWindow(const EmojiDb& emojiDb, bool darkBg, bool noCatList, bool noKwList,
                         const boost::optional<unsigned int>& selectedEmojiFlashPeriod);

signals:
    void emojiChosen(const Emoji& emoji, const boost::optional<Emoji::SkinTone>& skinTone,
                     bool removeVs16);
    void canceled();

public slots:
    void emojiDbChanged();

private:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void _setMainStyleSheet();
    void _buildUi(bool darkBg, bool noCatList, bool noKwList,
                  const boost::optional<unsigned int>& selectedEmojiFlashPeriod);
    QListWidget *_createCatListWidget();
    void _updateBottomLabels(const Emoji *emoji);
    void _updateInfoLabel(const Emoji *emoji);
    void _updateVersionLabel(const Emoji *emoji);
    void _updateKwLabel(const Emoji *emoji);
    void _findEmojis(const std::string& cat, const std::string& needles);
    void _acceptSelectedEmoji(const boost::optional<Emoji::SkinTone>& skinTone, bool removeVs16);
    void _acceptEmoji(const Emoji& emoji, const boost::optional<Emoji::SkinTone>& skinTone,
                      bool removeVs16);
    void _requestSelectedEmojiInfo();
    void _requestEmojiInfo(const Emoji& emoji);

private slots:
    void _searchTextChanged(const QString& text);
    void _catListItemSelectionChanged();
    void _catListItemClicked(QListWidgetItem *item);
    void _searchBoxUpKeyPressed();
    void _searchBoxRightKeyPressed(bool withCtrl);
    void _searchBoxDownKeyPressed();
    void _searchBoxLeftKeyPressed(bool withCtrl);
    void _searchBoxEnterKeyPressed(bool withShift);
    void _searchBoxF1KeyPressed(bool withShift);
    void _searchBoxF2KeyPressed(bool withShift);
    void _searchBoxF3KeyPressed(bool withShift);
    void _searchBoxF4KeyPressed(bool withShift);
    void _searchBoxF5KeyPressed(bool withShift);
    void _searchBoxF12KeyPressed();
    void _searchBoxPgUpKeyPressed();
    void _searchBoxPgDownKeyPressed();
    void _searchBoxHomeKeyPressed();
    void _searchBoxEndKeyPressed();
    void _searchBoxEscapeKeyPressed();
    void _emojiSelectionChanged(const Emoji *emoji);
    void _emojiClicked(const Emoji& emoji);
    void _emojiHoverEntered(const Emoji& emoji);
    void _emojiHoverLeaved(const Emoji& emoji);

private:
    const EmojiDb * const _emojiDb;
    QEmojisWidget *_wEmojis = nullptr;
    QListWidget *_wCatList = nullptr;
    QLabel *_wInfoLabel = nullptr;
    QLabel *_wVersionLabel = nullptr;
    QLabel *_wKwLabel = nullptr;
    QLineEdit *_wSearchBox = nullptr;
    bool _emojisWidgetBuilt = false;
    const Emoji *_selectedEmoji = nullptr;
};

} // namespace jome

#endif // _JOME_Q_JOME_WINDOW_HPP
