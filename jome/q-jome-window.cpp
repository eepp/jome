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

#include "q-jome-window.hpp"
#include "q-cat-list-widget-item.hpp"
#include "emojipedia.hpp"
#include "utils.hpp"

namespace jome {

QSearchBoxEventFilter::QSearchBoxEventFilter(QObject * const parent) :
    QObject {parent}
{
}

bool QSearchBoxEventFilter::eventFilter(QObject * const obj, QEvent * const event)
{
    if (event->type() != QEvent::KeyPress) {
        return QObject::eventFilter(obj, event);
    }

    const auto keyEvent = static_cast<const QKeyEvent *>(event);
    const bool withCtrl = keyEvent->modifiers() & Qt::ControlModifier;

    switch (keyEvent->key()) {
    case Qt::Key_Up:
        emit this->upKeyPressed();
        break;

    case Qt::Key_Right:
        emit this->rightKeyPressed(withCtrl);
        break;

    case Qt::Key_Down:
        emit this->downKeyPressed();
        break;

    case Qt::Key_Left:
        emit this->leftKeyPressed(withCtrl);
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

    case Qt::Key_F12:
        emit this->f12KeyPressed();
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

    case Qt::Key_Escape:
        emit this->escapeKeyPressed();
        break;

    case Qt::Key_C:
        if (withCtrl) {
            emit this->escapeKeyPressed();
        } else {
            return QObject::eventFilter(obj, event);
        }

        break;

    default:
        return QObject::eventFilter(obj, event);
    }

    return true;
}

QJomeWindow::QJomeWindow(const EmojiDb& emojiDb, const bool darkBg) :
    _emojiDb {&emojiDb}
{
    this->setWindowIcon(QIcon {QString {JOME_DATA_DIR} + "/icon.png"});
    this->setWindowTitle("jome");
    this->resize(800, 600);
    this->_setMainStyleSheet();
    this->_buildUi(darkBg);
}

void QJomeWindow::_setMainStyleSheet()
{
    static const char * const styleSheet =
        "* {"
        "  font-family: 'Hack', 'DejaVu Sans Mono', monospace;"
        "  font-size: 12px;"
        "  border: none;"
        "}"
        "QMainWindow {"
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
    QObject::connect(listWidget, &QListWidget::itemSelectionChanged, this,
                     &QJomeWindow::_catListItemSelectionChanged);
    QObject::connect(listWidget, &QListWidget::itemClicked, this,
                     &QJomeWindow::_catListItemClicked);
    return listWidget;
}

void QJomeWindow::_buildUi(const bool darkBg)
{
    _wSearchBox = new QLineEdit;
    QObject::connect(_wSearchBox, &QLineEdit::textChanged,
                     this, &QJomeWindow::_searchTextChanged);

    auto eventFilter = new QSearchBoxEventFilter {this};

    _wSearchBox->installEventFilter(eventFilter);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::upKeyPressed, this,
                     &QJomeWindow::_searchBoxUpKeyPressed);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::rightKeyPressed, this,
                     &QJomeWindow::_searchBoxRightKeyPressed);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::downKeyPressed, this,
                     &QJomeWindow::_searchBoxDownKeyPressed);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::leftKeyPressed, this,
                     &QJomeWindow::_searchBoxLeftKeyPressed);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::enterKeyPressed, this,
                     &QJomeWindow::_searchBoxEnterKeyPressed);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::f1KeyPressed, this,
                     &QJomeWindow::_searchBoxF1KeyPressed);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::f2KeyPressed, this,
                     &QJomeWindow::_searchBoxF2KeyPressed);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::f3KeyPressed, this,
                     &QJomeWindow::_searchBoxF3KeyPressed);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::f4KeyPressed, this,
                     &QJomeWindow::_searchBoxF4KeyPressed);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::f5KeyPressed, this,
                     &QJomeWindow::_searchBoxF5KeyPressed);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::f12KeyPressed, this,
                     &QJomeWindow::_searchBoxF12KeyPressed);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::pgUpKeyPressed, this,
                     &QJomeWindow::_searchBoxPgUpKeyPressed);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::pgDownKeyPressed, this,
                     &QJomeWindow::_searchBoxPgDownKeyPressed);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::homeKeyPressed, this,
                     &QJomeWindow::_searchBoxHomeKeyPressed);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::endKeyPressed, this,
                     &QJomeWindow::_searchBoxEndKeyPressed);
    QObject::connect(eventFilter, &QSearchBoxEventFilter::escapeKeyPressed, this,
                     &QJomeWindow::_searchBoxEscapeKeyPressed);

    auto mainVbox = new QVBoxLayout;

    mainVbox->setMargin(8);
    mainVbox->setSpacing(8);
    mainVbox->addWidget(_wSearchBox);
    _wEmojis = new QEmojisWidget {nullptr, *_emojiDb, darkBg};
    QObject::connect(_wEmojis, &QEmojisWidget::selectionChanged, this,
                     &QJomeWindow::_emojiSelectionChanged);
    QObject::connect(_wEmojis, &QEmojisWidget::emojiClicked, this, &QJomeWindow::_emojiClicked);
    QObject::connect(_wEmojis, &QEmojisWidget::emojiHoverEntered, this,
                     &QJomeWindow::_emojiHoverEntered);
    QObject::connect(_wEmojis, &QEmojisWidget::emojiHoverLeaved, this,
                     &QJomeWindow::_emojiHoverLeaved);
    _wCatList = this->_createCatListWidget();
    this->_wCatList->setCurrentRow(0);

    {
        const auto emojisHbox = new QHBoxLayout;

        emojisHbox->setMargin(0);
        emojisHbox->setSpacing(8);
        emojisHbox->addWidget(_wEmojis);
        emojisHbox->addWidget(_wCatList);
        mainVbox->addLayout(emojisHbox);
    }

    {
        const auto centralWidget = new QWidget;

        centralWidget->setLayout(mainVbox);
        this->setCentralWidget(centralWidget);
    }

    const auto infoHbox = new QHBoxLayout;

    _wInfoLabel = new QLabel {""};
    infoHbox->addWidget(_wInfoLabel);
    _wVersionLabel = new QLabel {""};
    _wVersionLabel->setFixedWidth(80);
    _wVersionLabel->setAlignment(Qt::AlignRight);
    infoHbox->addWidget(_wVersionLabel);
    mainVbox->addLayout(infoHbox);
}

void QJomeWindow::showEvent(QShowEvent * const event)
{
    QMainWindow::showEvent(event);

    if (!_emojisWidgetBuilt) {
        _wEmojis->rebuild();
        _emojisWidgetBuilt = true;
    }

    _wEmojis->showAllEmojis();
    _wSearchBox->clear();
    _wSearchBox->setFocus();
}

void QJomeWindow::closeEvent(QCloseEvent * const event)
{
    event->ignore();
    this->hide();
    emit this->canceled();
}

void QJomeWindow::_findEmojis(const std::string& cat, const std::string& needles)
{
    std::vector<const Emoji *> results;

    _emojiDb->findEmojis(cat, needles, results);
    _wEmojis->showFindResults(results);
}

void QJomeWindow::_searchTextChanged(const QString& text)
{
    if (text.isEmpty()) {
        _wEmojis->showAllEmojis();
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
    if (!_wEmojis->showingAllEmojis()) {
        return;
    }

    auto selectedItems = _wCatList->selectedItems();

    if (selectedItems.isEmpty()) {
        return;
    }

    const auto& item = static_cast<const QCatListWidgetItem&>(*selectedItems[0]);

    _wEmojis->scrollToCat(item.cat());
}

void QJomeWindow::_catListItemClicked(QListWidgetItem * const)
{
    this->_catListItemSelectionChanged();
}

void QJomeWindow::_searchBoxUpKeyPressed()
{
    _wEmojis->selectPreviousRow();
}

void QJomeWindow::_searchBoxRightKeyPressed(const bool withCtrl)
{
    _wEmojis->selectNext(withCtrl ? 5 : 1);
}

void QJomeWindow::_searchBoxDownKeyPressed()
{
    _wEmojis->selectNextRow();
}

void QJomeWindow::_searchBoxLeftKeyPressed(const bool withCtrl)
{
    _wEmojis->selectPrevious(withCtrl ? 5 : 1);
}

void QJomeWindow::_searchBoxPgUpKeyPressed()
{
    _wEmojis->selectPreviousRow(10);
}

void QJomeWindow::_searchBoxPgDownKeyPressed()
{
    _wEmojis->selectNextRow(10);
}

void QJomeWindow::_searchBoxHomeKeyPressed()
{
    _wEmojis->selectFirst();
}

void QJomeWindow::_searchBoxEndKeyPressed()
{
    _wEmojis->selectLast();
}

void QJomeWindow::_searchBoxEscapeKeyPressed()
{
    this->hide();
    emit this->canceled();
}

void QJomeWindow::_searchBoxEnterKeyPressed()
{
    this->_acceptSelectedEmoji(boost::none);
}

void QJomeWindow::_searchBoxF1KeyPressed()
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::Light);
}

void QJomeWindow::_searchBoxF2KeyPressed()
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::MediumLight);
}

void QJomeWindow::_searchBoxF3KeyPressed()
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::Medium);
}

void QJomeWindow::_searchBoxF4KeyPressed()
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::MediumDark);
}

void QJomeWindow::_searchBoxF5KeyPressed()
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::Dark);
}

void QJomeWindow::_searchBoxF12KeyPressed()
{
    this->_requestSelectedEmojiInfo();
}

void QJomeWindow::_emojiSelectionChanged(const Emoji * const emoji)
{
    _selectedEmoji = emoji;
    this->_updateBottomLabels(emoji);
}

void QJomeWindow::_emojiClicked(const Emoji& emoji)
{
    this->_acceptEmoji(emoji, boost::none);
}

void QJomeWindow::_emojiHoverEntered(const Emoji& emoji)
{
    this->_updateBottomLabels(&emoji);
}

void QJomeWindow::_emojiHoverLeaved(const Emoji&)
{
    this->_updateBottomLabels(_selectedEmoji);
}

void QJomeWindow::_acceptSelectedEmoji(const boost::optional<Emoji::SkinTone>& skinTone)
{
    if (_selectedEmoji) {
        this->_acceptEmoji(*_selectedEmoji, skinTone);
    }
}

void QJomeWindow::_acceptEmoji(const Emoji& emoji,
                               const boost::optional<Emoji::SkinTone>& skinTone)
{
    if (skinTone && !emoji.hasSkinToneSupport()) {
        return;
    }

    emit this->emojiChosen(emoji, skinTone);
}

void QJomeWindow::_requestSelectedEmojiInfo()
{
    if (_selectedEmoji) {
        this->_requestEmojiInfo(*_selectedEmoji);
    }
}

void QJomeWindow::_requestEmojiInfo(const Emoji& emoji)
{
    gotoEmojipediaPage(emoji);
}

void QJomeWindow::_updateBottomLabels(const Emoji * const emoji)
{
    this->_updateInfoLabel(emoji);
    this->_updateVersionLabel(emoji);
}

void QJomeWindow::_updateInfoLabel(const Emoji * const emoji)
{
    QString text;

    if (emoji) {
        text += "<b>";
        text += emoji->name().c_str();
        text += "</b> <span style=\"color: #999\">(";

        for (const auto codepoint : emoji->codepoints()) {
            text += QString::number(codepoint, 16) + ", ";
        }

        text.truncate(text.size() - 2);
        text += ")</span>";
    }

    _wInfoLabel->setText(text);
}

void QJomeWindow::_updateVersionLabel(const Emoji * const emoji)
{
    _wVersionLabel->setText(call([emoji] {
        QString text;

        if (emoji) {
            text += "<span style=\"color: #2ecc71\">Emoji <b>";

            text += call([emoji] {
                switch (emoji->version()) {
                case EmojiVersion::V_0_6:
                    return "0.6";

                case EmojiVersion::V_0_7:
                    return "0.7";

                case EmojiVersion::V_1_0:
                    return "1.0";

                case EmojiVersion::V_2_0:
                    return "2.0";

                case EmojiVersion::V_3_0:
                    return "3.0";

                case EmojiVersion::V_4_0:
                    return "4.0";

                case EmojiVersion::V_5_0:
                    return "5.0";

                case EmojiVersion::V_11_0:
                    return "11.0";

                case EmojiVersion::V_12_0:
                    return "12.0";

                case EmojiVersion::V_12_1:
                    return "12.1";

                case EmojiVersion::V_13_0:
                    return "13.0";

                case EmojiVersion::V_13_1:
                    return "13.1";

                case EmojiVersion::V_14_0:
                    return "14.0";

                default:
                    std::abort();
                }
            });

            text += "</b></span>";
        }

        return text;
    }));
}

void QJomeWindow::emojiDbChanged()
{
    _wEmojis->rebuild();
    _wEmojis->showAllEmojis();
}

} // namespace jome
