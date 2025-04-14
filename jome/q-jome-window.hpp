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
#include "q-emoji-grid-widget.hpp"

namespace jome {

/*
 * An event filter for the find box which emits signals when the
 * user presses special navigation and action keys.
 */
class QFindBoxEventFilter final :
    public QObject
{
    Q_OBJECT

public:
    explicit QFindBoxEventFilter(QObject *parent);

private:
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

/*
 * The actual jome window.
 *
 * It has a pointer to the database to deal with initial emoji grid
 * construction and to find emojis.
 *
 * Once you build the window, the only relevant outcomes are:
 *
 * emojiChosen() signal:
 *     An emoji was chosen.
 *
 * cancelled() signal:
 *     The emoji picking operation was cancelled.
 *
 * Call emojiDbChanged() whenever you update the linked emoji database.
 */
class QJomeWindow final :
    public QMainWindow
{
    Q_OBJECT

public:
    /*
     * Builds a jome window to display the emojis of `emojiDb` with:
     *
     * • A dark background if `darkBg` is true.
     *
     * • No category list if `noCatList` is true.
     *
     * • No category labels within the emoji grid if `noCatLabels`
     *   is true.
     *
     * • No keyword list at the bottom if `noKwList` is true.
     *
     * • A selection square flashing period of
     *   `*selectedEmojiFlashPeriod` is set.
     */
    explicit QJomeWindow(const EmojiDb& emojiDb, bool darkBg, bool noCatList, bool noCatLabels,
                         bool noKwList,
                         const boost::optional<unsigned int>& selectedEmojiFlashPeriod);

signals:
    /*
     * Emoji `emoji` was chosen, possibly with the skin tone `skinTone`,
     * and with a forced removal of VS-16 codepoints if `removeVs16`
     * is true.
     */
    void emojiChosen(const Emoji& emoji, const boost::optional<Emoji::SkinTone>& skinTone,
                     bool removeVs16);

    /*
     * Emoji picking operation was cancelled.
     */
    void cancelled();

public slots:
    /*
     * The linked emoji database changed behind the scenes.
     */
    void emojiDbChanged();

private:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void _setMainStyleSheet();
    void _buildUi(bool darkBg, bool noCatList, bool noCatLabels, bool noKwList,
                  const boost::optional<unsigned int>& selectedEmojiFlashPeriod);
    QListWidget *_createCatListWidget();
    void _updateBottomLabels(const Emoji *emoji);
    void _updateInfoLabel(const Emoji *emoji);
    void _updateVersionLabel(const Emoji *emoji);
    void _updateKwLabel(const Emoji *emoji);
    void _findEmojis(const QString& cat, const QString& needles);
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
    void _emojiClicked(const Emoji& emoji, bool withShift);
    void _emojiHoverEntered(const Emoji& emoji);
    void _emojiHoverLeaved(const Emoji& emoji);

private:
    const EmojiDb * const _emojiDb;
    QEmojiGridWidget *_wEmojiGrid = nullptr;
    QListWidget *_wCatList = nullptr;
    QLabel *_wInfoLabel = nullptr;
    QLabel *_wVersionLabel = nullptr;
    QLabel *_wKwLabel = nullptr;
    QLineEdit *_wFindBox = nullptr;
    bool _emojisWidgetBuilt = false;
    const Emoji *_selectedEmoji = nullptr;
};

} // namespace jome

#endif // _JOME_Q_JOME_WINDOW_HPP
