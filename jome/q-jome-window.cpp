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
#include <boost/algorithm/string.hpp>

#include "q-jome-window.hpp"
#include "q-cat-list-widget-item.hpp"

namespace jome {

QJomeWindow::QJomeWindow(const EmojiDb& emojiDb) :
    QDialog {},
    _emojiDb {&emojiDb},
    _emojiImages {emojiDb}
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

    auto bottomHbox = new QHBoxLayout;

    bottomHbox->setMargin(0);
    bottomHbox->setSpacing(8);
    bottomHbox->addWidget(_wEmojisGraphicsView);
    bottomHbox->addWidget(_wCatList);
    mainVbox->addLayout(bottomHbox);
    this->setLayout(mainVbox);
    _allEmojisGraphicsSceneSelectedItem = this->_createSelectedGraphicsItem();
    _allEmojisGraphicsScene.addItem(_allEmojisGraphicsSceneSelectedItem);
    _findEmojisGraphicsSceneSelectedItem = this->_createSelectedGraphicsItem();
    _findEmojisGraphicsScene.addItem(_findEmojisGraphicsSceneSelectedItem);
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

    _curEmojiGraphicsItems[0]->pos();

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

    selectedItem->show();
    selectedItem->setPos(_curEmojiGraphicsItems[*index]->pos());
}

} // namespace jome
