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
        "QScrollArea {"
        "  background-color: #f8f8f8;"
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

    listWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    QObject::connect(listWidget, SIGNAL(itemSelectionChanged()),
                     this, SLOT(_catListItemSelectionChanged()));
    QObject::connect(listWidget, SIGNAL(itemClicked(QListWidgetItem *)),
                     this, SLOT(_catListItemClicked(QListWidgetItem *)));
    return listWidget;
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
    _wEmojisArea = new QScrollArea;
    _wEmojisArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _wEmojisArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    _wCatList = this->_createCatListWidget();

    auto bottomHbox = new QHBoxLayout;

    bottomHbox->setMargin(0);
    bottomHbox->setSpacing(8);
    bottomHbox->addWidget(_wEmojisArea);
    bottomHbox->addWidget(_wCatList);
    mainVbox->addLayout(bottomHbox);
    this->setLayout(mainVbox);
}

void QJomeWindow::showEvent(QShowEvent * const event)
{
    QDialog::showEvent(event);
    _wCatList->setMinimumWidth(_wCatList->sizeHintForColumn(0));
    this->_showAllEmojis();
    this->_wCatList->setCurrentRow(0);
}

void QJomeWindow::_showAllEmojis()
{
    if (_wAllEmojisAreaWidget) {
        _wEmojisArea->setWidget(_wAllEmojisAreaWidget);
        return;
    }

    std::unordered_map<const QWidget *, const EmojiCat *> wLabelToCat;
    _wAllEmojisAreaWidget = new QWidget;

    _wAllEmojisAreaWidget->setStyleSheet("background-color: transparent;");

    auto vbox = new QVBoxLayout {_wAllEmojisAreaWidget};

    vbox->setMargin(8);
    vbox->setSpacing(8);

    const auto availWidth = static_cast<unsigned int>(_wEmojisArea->viewport()->width() - 16);

    for (const auto& cat : _emojiDb->cats()) {
        auto lbl = new QLabel {QString::fromStdString(cat->name())};

        lbl->setStyleSheet("font-weight: bold;");
        wLabelToCat[lbl] = cat.get();
        vbox->addWidget(lbl);

        auto hbox = new QHBoxLayout;

        hbox->setMargin(0);

        auto grid = new QGridLayout;

        grid->setSpacing(8);
        grid->setMargin(0);

        auto col = 0U;
        auto row = 0U;

        for (const auto& emoji : cat->emojis()) {
            auto lbl = new QLabel {""};

            lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            lbl->resize(32, 32);
            lbl->setPixmap(_emojiImages.pixmapForEmoji(*emoji));
            grid->addWidget(lbl, row, col);
            col += 1;

            if ((col + 1) * (32 + 8) >= availWidth) {
                col = 0;
                row += 1;
            }
        }

        hbox->addLayout(grid);
        hbox->addStretch();
        vbox->addLayout(hbox);
        vbox->addSpacing(16);
    }

    _wEmojisArea->setWidget(_wAllEmojisAreaWidget);

    // keep vertical positions now
    for (const auto& pair : wLabelToCat) {
        const auto y = pair.first->pos().y();

        _catVertPositions[pair.second] = y;
    }
}

void QJomeWindow::_findEmojis(const std::string& cat,
                              const std::string& needles)
{
    if (_wAllEmojisAreaWidget) {
        _wEmojisArea->takeWidget();
    }

    auto areaWidget = new QWidget;

    areaWidget->setStyleSheet("background-color: transparent;");

    std::vector<const Emoji *> results;

    _emojiDb->findEmojis(cat, needles, results);

    if (results.empty()) {
        _wEmojisArea->setWidget(areaWidget);
    }

    auto vbox = new QVBoxLayout {areaWidget};

    vbox->setMargin(8);
    vbox->setSpacing(8);

    const auto availWidth = static_cast<unsigned int>(_wEmojisArea->viewport()->width() - 16);

    auto hbox = new QHBoxLayout;

    hbox->setMargin(0);

    auto grid = new QGridLayout;

    grid->setSpacing(8);
    grid->setMargin(0);

    auto col = 0U;
    auto row = 0U;

    for (const auto& emoji : results) {
        auto lbl = new QLabel {""};

        lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        lbl->resize(32, 32);
        lbl->setPixmap(_emojiImages.pixmapForEmoji(*emoji));
        grid->addWidget(lbl, row, col);
        col += 1;

        if ((col + 1) * (32 + 8) >= availWidth) {
            col = 0;
            row += 1;
        }
    }

    hbox->addLayout(grid);
    hbox->addStretch();
    vbox->addLayout(hbox);
    _wEmojisArea->setWidget(areaWidget);
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
    if (_wEmojisArea->widget() != _wAllEmojisAreaWidget) {
        return;
    }

    auto selectedItems = _wCatList->selectedItems();

    if (selectedItems.isEmpty()) {
        return;
    }

    const auto& item = static_cast<const QCatListWidgetItem&>(*selectedItems[0]);
    const auto y = std::max(0, _catVertPositions[&item.cat()] - 8);

    _wEmojisArea->verticalScrollBar()->setValue(y);
}

void QJomeWindow::_catListItemClicked(QListWidgetItem * const item)
{
    this->_catListItemSelectionChanged();
}

} // namespace jome
