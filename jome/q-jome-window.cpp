/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
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

#include "q-jome-window.hpp"
#include "q-cat-list-widget-item.hpp"

namespace jome {

QSearchBoxEventFilter::QSearchBoxEventFilter(QObject * const parent) :
    QObject {parent}
{
}

bool QSearchBoxEventFilter::eventFilter(QObject * const obj,
                                        QEvent * const event)
{
    if (event->type() != QEvent::KeyPress) {
        return QObject::eventFilter(obj, event);
    }

    auto keyEvent = static_cast<const QKeyEvent *>(event);

    switch (keyEvent->key()) {
    case Qt::Key_Up:
        emit this->upKeyPressed();
        break;

    case Qt::Key_Right:
        emit this->rightKeyPressed();
        break;

    case Qt::Key_Down:
        emit this->downKeyPressed();
        break;

    case Qt::Key_Left:
        emit this->leftKeyPressed();
        break;

    case Qt::Key_F1:
        emit this->f1KeyPressed();
        break;

    case Qt::Key_F2:
        emit this->f2KeyPressed();
        break;

    case Qt::Key_F3:
        emit this->f3KeyPressed();
        break;

    case Qt::Key_F4:
        emit this->f4KeyPressed();
        break;

    case Qt::Key_F5:
        emit this->f5KeyPressed();
        break;

    case Qt::Key_PageUp:
        emit this->pgUpKeyPressed();
        break;

    case Qt::Key_PageDown:
        emit this->pgDownKeyPressed();
        break;

    case Qt::Key_Home:
        emit this->homeKeyPressed();
        break;

    case Qt::Key_End:
        emit this->endKeyPressed();
        break;

    case Qt::Key_Enter:
    case Qt::Key_Return:
        emit this->enterKeyPressed();
        break;

    default:
        return QObject::eventFilter(obj, event);
    }

    return true;
}

QJomeWindow::QJomeWindow(const EmojiDb& emojiDb,
                         const EmojiChosenFunc& emojiChosenFunc) :
    QDialog {},
    _emojiDb {&emojiDb},
    _emojiImages {emojiDb},
    _emojiChosenFunc {emojiChosenFunc}
{
    this->setWindowTitle("jome");
    this->setFixedSize(800, 600);
    this->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    this->_setMainStyleSheet();
    this->_buildUi();
}

void QJomeWindow::_setMainStyleSheet()
{
    static const char * const styleSheet =
        "* {"
        "  font-family: 'Hack', 'DejaVu Sans Mono', monospace;"
        "  font-size: 12px;"
        "  border: none;"
        "}"
        "QDialog {"
        "  background-color: #333;"
        "}"
        "QLineEdit {"
        "  background-color: rgba(0, 0, 0, 0.2);"
        "  color: #f0f0f0;"
        "  font-weight: bold;"
        "  font-size: 14px;"
        "  border-bottom: 2px solid #ff3366;"
        "  padding: 4px;"
        "}"
        "QListWidget {"
        "  background-color: transparent;"
        "  color: #e0e0e0;"
        "}"
        "QScrollBar:vertical {"
        "  border: none;"
        "  background-color: #666;"
        "  width: 8px;"
        "  margin: 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "  border: none;"
        "  background-color: #999;"
        "  min-height: 16px;"
        "}"
        "QScrollBar::add-line:vertical,"
        "QScrollBar::sub-line:vertical {"
        "  height: 0;"
        "}"
        "QLabel {"
        "  color: #ff3366;"
        "}";

    this->setStyleSheet(styleSheet);
}

QListWidget *QJomeWindow::_createCatListWidget()
{
    auto listWidget = new QListWidget;

    for (const auto& cat : _emojiDb->cats()) {
        listWidget->addItem(new QCatListWidgetItem {*cat});
    }

    listWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    listWidget->setFixedWidth(220);
    QObject::connect(listWidget, SIGNAL(itemSelectionChanged()),
                     this, SLOT(_catListItemSelectionChanged()));
    QObject::connect(listWidget, SIGNAL(itemClicked(QListWidgetItem *)),
                     this, SLOT(_catListItemClicked(QListWidgetItem *)));
    return listWidget;
}

void QJomeWindow::_setGraphicsSceneStyle(QGraphicsScene& gs)
{
    gs.setBackgroundBrush(QColor {"#f8f8f8"});
}

void QJomeWindow::_buildUi()
{
    auto searchBox = new QLineEdit;

    QObject::connect(searchBox, SIGNAL(textChanged(const QString&)),
                     this, SLOT(_searchTextChanged(const QString&)));

    auto eventFilter = new QSearchBoxEventFilter {this};

    searchBox->installEventFilter(eventFilter);
    QObject::connect(eventFilter, SIGNAL(upKeyPressed()),
                     this, SLOT(_searchBoxUpKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(rightKeyPressed()),
                     this, SLOT(_searchBoxRightKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(downKeyPressed()),
                     this, SLOT(_searchBoxDownKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(leftKeyPressed()),
                     this, SLOT(_searchBoxLeftKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(enterKeyPressed()),
                     this, SLOT(_searchBoxEnterKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(f1KeyPressed()),
                     this, SLOT(_searchBoxF1KeyPressed()));
    QObject::connect(eventFilter, SIGNAL(f2KeyPressed()),
                     this, SLOT(_searchBoxF2KeyPressed()));
    QObject::connect(eventFilter, SIGNAL(f3KeyPressed()),
                     this, SLOT(_searchBoxF3KeyPressed()));
    QObject::connect(eventFilter, SIGNAL(f4KeyPressed()),
                     this, SLOT(_searchBoxF4KeyPressed()));
    QObject::connect(eventFilter, SIGNAL(f5KeyPressed()),
                     this, SLOT(_searchBoxF5KeyPressed()));
    QObject::connect(eventFilter, SIGNAL(pgUpKeyPressed()),
                     this, SLOT(_searchBoxPgUpKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(pgDownKeyPressed()),
                     this, SLOT(_searchBoxPgDownKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(homeKeyPressed()),
                     this, SLOT(_searchBoxHomeKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(endKeyPressed()),
                     this, SLOT(_searchBoxEndKeyPressed()));

    auto mainVbox = new QVBoxLayout;

    mainVbox->setMargin(8);
    mainVbox->setSpacing(8);
    mainVbox->addWidget(searchBox);
    _wEmojisGraphicsView = new QGraphicsView();
    _wEmojisGraphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    _wEmojisGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    _wEmojisGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->_setGraphicsSceneStyle(_allEmojisGraphicsScene);
    this->_setGraphicsSceneStyle(_findEmojisGraphicsScene);
    _wCatList = this->_createCatListWidget();
    this->_wCatList->setCurrentRow(0);

    auto emojisHbox = new QHBoxLayout;

    emojisHbox->setMargin(0);
    emojisHbox->setSpacing(8);
    emojisHbox->addWidget(_wEmojisGraphicsView);
    emojisHbox->addWidget(_wCatList);
    mainVbox->addLayout(emojisHbox);
    this->setLayout(mainVbox);
    _allEmojisGraphicsSceneSelectedItem = this->_createSelectedGraphicsItem();
    _allEmojisGraphicsScene.addItem(_allEmojisGraphicsSceneSelectedItem);
    _findEmojisGraphicsSceneSelectedItem = this->_createSelectedGraphicsItem();
    _findEmojisGraphicsScene.addItem(_findEmojisGraphicsSceneSelectedItem);

    _wInfoLabel = new QLabel {""};
    mainVbox->addWidget(_wInfoLabel);
}

void QJomeWindow::showEvent(QShowEvent * const event)
{
    QDialog::showEvent(event);
    this->_buildAllEmojisGraphicsScene();
    this->_showAllEmojis();
}

void QJomeWindow::_buildAllEmojisGraphicsScene()
{
    if (_allEmojisGraphicsSceneBuilt) {
        return;
    }

    qreal y = 8.;

    _allEmojisGraphicsScene.setSceneRect(0., 0.,
                                         static_cast<qreal>(_wEmojisGraphicsView->width()) - 8., 0.);
    QFont font {"Hack, DejaVu Sans Mono, monospace", 10, QFont::Bold};

    for (const auto& cat : _emojiDb->cats()) {
        auto item = _allEmojisGraphicsScene.addText(QString::fromStdString(cat->name()),
                                                    font);

        item->setPos(8., y);
        _catVertPositions[cat.get()] = y;
        y += 24.;
        this->_addEmojisToGraphicsScene(cat->emojis(),
                                        _allEmojiGraphicsItems,
                                        _allEmojisGraphicsScene, y);
        y += 8.;
    }

    y -= 8.;
    _allEmojisGraphicsScene.setSceneRect(0., 0.,
                                         static_cast<qreal>(_wEmojisGraphicsView->width()) - 8., y);
    _allEmojisGraphicsSceneBuilt = true;
}

void QJomeWindow::_showAllEmojis()
{
    _curEmojiGraphicsItems = _allEmojiGraphicsItems;
    _wEmojisGraphicsView->setScene(&_allEmojisGraphicsScene);
    this->_selectEmojiGraphicsItem(0);
}

void QJomeWindow::_findEmojis(const std::string& cat,
                              const std::string& needles)
{
    std::vector<const Emoji *> results;

    _findEmojisGraphicsScene.removeItem(_findEmojisGraphicsSceneSelectedItem);
    _findEmojisGraphicsScene.clear();
    _findEmojisGraphicsScene.addItem(_findEmojisGraphicsSceneSelectedItem);
    _curEmojiGraphicsItems.clear();
    _emojiDb->findEmojis(cat, needles, results);
    qreal y = 0.;

    _findEmojisGraphicsScene.setSceneRect(0., 0.,
                                          static_cast<qreal>(_wEmojisGraphicsView->width()) - 8., 0.);

    if (!results.empty()) {
        y = 8.;
        this->_addEmojisToGraphicsScene(results, _curEmojiGraphicsItems,
                                        _findEmojisGraphicsScene, y);
    }

    _findEmojisGraphicsScene.setSceneRect(0., 0.,
                                          static_cast<qreal>(_wEmojisGraphicsView->width()) - 8., y);
    _wEmojisGraphicsView->setScene(&_findEmojisGraphicsScene);

    if (results.empty()) {
        this->_selectEmojiGraphicsItem(boost::none);
    } else {
        this->_selectEmojiGraphicsItem(0);
    }
}

void QJomeWindow::_searchTextChanged(const QString& text)
{
    if (text.isEmpty()) {
        this->_showAllEmojis();
        return;
    }

    std::vector<std::string> parts;
    const std::string textStr {text.toUtf8().constData()};

    boost::split(parts, textStr, boost::is_any_of("/"));

    if (parts.size() != 2) {
        this->_findEmojis("", textStr);
        return;
    }

    this->_findEmojis(parts[0], parts[1]);
}

void QJomeWindow::_catListItemSelectionChanged()
{
    if (_wEmojisGraphicsView->scene() != &_allEmojisGraphicsScene) {
        return;
    }

    auto selectedItems = _wCatList->selectedItems();

    if (selectedItems.isEmpty()) {
        return;
    }

    const auto& item = static_cast<const QCatListWidgetItem&>(*selectedItems[0]);
    const auto y = std::max(0., _catVertPositions[&item.cat()] - 8);

    _wEmojisGraphicsView->verticalScrollBar()->setValue(static_cast<int>(y));
}

void QJomeWindow::_catListItemClicked(QListWidgetItem * const item)
{
    this->_catListItemSelectionChanged();
}

QGraphicsPixmapItem *QJomeWindow::_createSelectedGraphicsItem()
{
    const auto path = std::string {JOME_DATA_DIR} + "/sel.png";

    QImage image {QString::fromStdString(path)};
    auto graphicsItem = new QGraphicsPixmapItem {QPixmap::fromImage(std::move(image))};

    graphicsItem->hide();
    graphicsItem->setEnabled(false);
    graphicsItem->setZValue(1000.);
    return graphicsItem;
}

void QJomeWindow::_selectEmojiGraphicsItem(const boost::optional<unsigned int>& index)
{
    QGraphicsPixmapItem *selectedItem;

    if (_wEmojisGraphicsView->scene() == &_allEmojisGraphicsScene) {
        selectedItem = _allEmojisGraphicsSceneSelectedItem;
    } else {
        selectedItem = _findEmojisGraphicsSceneSelectedItem;
    }

    _selectedEmojiGraphicsItemIndex = index;

    if (!index) {
        selectedItem->hide();
        return;
    }

    assert(*index < _curEmojiGraphicsItems.size());

    const auto& emojiGraphicsItem = *_curEmojiGraphicsItems[*index];

    selectedItem->show();
    selectedItem->setPos(emojiGraphicsItem.pos());

    if (*index == 0) {
        _wEmojisGraphicsView->verticalScrollBar()->setValue(0);
    } else {
        const auto candY = selectedItem->pos().y() + 16. -
                           static_cast<qreal>(_wEmojisGraphicsView->height()) / 2.;
        const auto y = std::max(0., candY);
        _wEmojisGraphicsView->verticalScrollBar()->setValue(static_cast<int>(y));
    }

    this->_updateInfoLabel(emojiGraphicsItem.emoji());
}

void QJomeWindow::_searchBoxUpKeyPressed()
{
    if (!_selectedEmojiGraphicsItemIndex) {
        return;
    }

    const auto& selectedEmojiGraphicsItem = *_curEmojiGraphicsItems[*_selectedEmojiGraphicsItemIndex];
    const auto curX = selectedEmojiGraphicsItem.pos().x();

    for (auto i = static_cast<int>(*_selectedEmojiGraphicsItemIndex) - 1; i >= 0; --i) {
        const auto& emojiGraphicsItem = *_curEmojiGraphicsItems[i];

        if (emojiGraphicsItem.pos().x() == curX) {
            this->_selectEmojiGraphicsItem(static_cast<unsigned int>(i));
            return;
        }
    }
}

void QJomeWindow::_searchBoxRightKeyPressed()
{
    if (!_selectedEmojiGraphicsItemIndex) {
        return;
    }

    if (*_selectedEmojiGraphicsItemIndex + 1 == _curEmojiGraphicsItems.size()) {
        return;
    }

    this->_selectEmojiGraphicsItem(*_selectedEmojiGraphicsItemIndex + 1);
}

void QJomeWindow::_searchBoxDownKeyPressed()
{
    if (!_selectedEmojiGraphicsItemIndex) {
        return;
    }

    const auto& selectedEmojiGraphicsItem = *_curEmojiGraphicsItems[*_selectedEmojiGraphicsItemIndex];
    const auto curX = selectedEmojiGraphicsItem.pos().x();

    for (auto i = *_selectedEmojiGraphicsItemIndex + 1; i < _curEmojiGraphicsItems.size(); ++i) {
        const auto& emojiGraphicsItem = *_curEmojiGraphicsItems[i];

        if (emojiGraphicsItem.pos().x() == curX) {
            this->_selectEmojiGraphicsItem(i);
            return;
        }
    }
}

void QJomeWindow::_searchBoxLeftKeyPressed()
{
    if (!_selectedEmojiGraphicsItemIndex) {
        return;
    }

    if (*_selectedEmojiGraphicsItemIndex == 0) {
        return;
    }

    this->_selectEmojiGraphicsItem(*_selectedEmojiGraphicsItemIndex - 1);
}

void QJomeWindow::_searchBoxPgUpKeyPressed()
{
    for (auto i = 0U; i < 10; ++i) {
        this->_searchBoxUpKeyPressed();
    }
}

void QJomeWindow::_searchBoxPgDownKeyPressed()
{
    for (auto i = 0U; i < 10; ++i) {
        this->_searchBoxDownKeyPressed();
    }
}

void QJomeWindow::_searchBoxHomeKeyPressed()
{
    if (_curEmojiGraphicsItems.empty()) {
        return;
    }

    this->_selectEmojiGraphicsItem(0);
}

void QJomeWindow::_searchBoxEndKeyPressed()
{
    if (_curEmojiGraphicsItems.empty()) {
        return;
    }

    this->_selectEmojiGraphicsItem(_curEmojiGraphicsItems.size() - 1);
}

void QJomeWindow::_searchBoxEnterKeyPressed()
{
    this->_acceptEmoji(Emoji::SkinTone::NONE);
}

void QJomeWindow::_searchBoxF1KeyPressed()
{
    this->_acceptEmoji(Emoji::SkinTone::LIGHT);
}

void QJomeWindow::_searchBoxF2KeyPressed()
{
    this->_acceptEmoji(Emoji::SkinTone::MEDIUM_LIGHT);
}

void QJomeWindow::_searchBoxF3KeyPressed()
{
    this->_acceptEmoji(Emoji::SkinTone::MEDIUM);
}

void QJomeWindow::_searchBoxF4KeyPressed()
{
    this->_acceptEmoji(Emoji::SkinTone::MEDIUM_DARK);
}

void QJomeWindow::_searchBoxF5KeyPressed()
{
    this->_acceptEmoji(Emoji::SkinTone::DARK);
}

void QJomeWindow::_acceptEmoji(const Emoji::SkinTone skinTone)
{
    const auto selectedEmoji = this->_selectedEmoji();

    if (selectedEmoji) {
        _emojiChosenFunc(*selectedEmoji, skinTone);
    }

    this->done(0);
}

void QJomeWindow::_updateInfoLabel(const Emoji& emoji)
{
    QString text;

    text += "<b>";
    text += emoji.name().c_str();
    text += "</b> <span style=\"color: #999\">(";

    for (const auto codepoint : emoji.codepoints()) {
        text += QString::number(codepoint, 16) + ", ";
    }

    text.truncate(text.size() - 2);
    text += ")</span>";
    _wInfoLabel->setText(text);
}

const Emoji *QJomeWindow::_selectedEmoji()
{
    if (!_selectedEmojiGraphicsItemIndex) {
        return nullptr;
    }

    return &_curEmojiGraphicsItems[*_selectedEmojiGraphicsItemIndex]->emoji();
}

} // namespace jome
