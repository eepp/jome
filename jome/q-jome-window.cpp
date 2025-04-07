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
#include <QStringList>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QPalette>
#include <boost/algorithm/string.hpp>
#include <boost/optional/optional.hpp>

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
    const bool withShift = keyEvent->modifiers() & Qt::ShiftModifier;

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
        emit this->f1KeyPressed(withShift);
        break;

    case Qt::Key_F2:
        emit this->f2KeyPressed(withShift);
        break;

    case Qt::Key_F3:
        emit this->f3KeyPressed(withShift);
        break;

    case Qt::Key_F4:
        emit this->f4KeyPressed(withShift);
        break;

    case Qt::Key_F5:
        emit this->f5KeyPressed(withShift);
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
        emit this->enterKeyPressed(withShift);
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

QJomeWindow::QJomeWindow(const EmojiDb& emojiDb, const bool darkBg, const bool noCatList,
                         const bool noKwList,
                         const boost::optional<unsigned int>& selectedEmojiFlashPeriod) :
    _emojiDb {&emojiDb}
{
    this->setWindowIcon(QIcon {qFmtFormat("{}/icon.png", JOME_DATA_DIR)});
    this->setWindowTitle("jome");
    this->resize(800, 600);
    this->_setMainStyleSheet();
    this->_buildUi(darkBg, noCatList, noKwList, selectedEmojiFlashPeriod);
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
        "QListWidget::item:selected {"
        "  background-color: #ff3366;"
        "  color: #fff;"
        "  font-weight: bold;"
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
    QObject::connect(listWidget, &QListWidget::itemSelectionChanged, this,
                     &QJomeWindow::_catListItemSelectionChanged);
    QObject::connect(listWidget, &QListWidget::itemClicked, this,
                     &QJomeWindow::_catListItemClicked);
    return listWidget;
}

namespace {

void setQLabelFgColor(QLabel& widget, const QColor& color)
{
    auto palette = widget.palette();

    palette.setColor(QPalette::WindowText, color);
    widget.setPalette(palette);
}

} // namespace

void QJomeWindow::_buildUi(const bool darkBg, const bool noCatList, const bool noKwList,
                           const boost::optional<unsigned int>& selectedEmojiFlashPeriod)
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
    _wEmojis = new QEmojisWidget {nullptr, *_emojiDb, darkBg, selectedEmojiFlashPeriod};
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

    if (noCatList) {
        this->_wCatList->hide();
    }

    {
        const auto centralWidget = new QWidget;

        centralWidget->setLayout(mainVbox);
        this->setCentralWidget(centralWidget);
    }

    {
        const auto infoHbox = new QHBoxLayout;

        _wInfoLabel = new QLabel {""};
        setQLabelFgColor(*_wInfoLabel, "#ff3366");
        infoHbox->addWidget(_wInfoLabel);
        _wVersionLabel = new QLabel {""};
        setQLabelFgColor(*_wVersionLabel, "#2ecc71");
        _wVersionLabel->setFixedWidth(150);
        _wVersionLabel->setAlignment(Qt::AlignRight);
        infoHbox->addWidget(_wVersionLabel);
        mainVbox->addLayout(infoHbox);
    }

    {
        _wKwLabel = new QLabel {""};
        setQLabelFgColor(*_wKwLabel, "#f39c12");
        _wKwLabel->setWordWrap(false);
        _wKwLabel->setTextInteractionFlags(Qt::NoTextInteraction);
        _wKwLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        mainVbox->addWidget(_wKwLabel);

        if (noKwList) {
            _wKwLabel->hide();
        }
    }
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

void QJomeWindow::_searchBoxEnterKeyPressed(const bool withShift)
{
    this->_acceptSelectedEmoji(boost::none, withShift);
}

void QJomeWindow::_searchBoxF1KeyPressed(const bool withShift)
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::Light, withShift);
}

void QJomeWindow::_searchBoxF2KeyPressed(const bool withShift)
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::MediumLight, withShift);
}

void QJomeWindow::_searchBoxF3KeyPressed(const bool withShift)
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::Medium, withShift);
}

void QJomeWindow::_searchBoxF4KeyPressed(const bool withShift)
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::MediumDark, withShift);
}

void QJomeWindow::_searchBoxF5KeyPressed(const bool withShift)
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::Dark, withShift);
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
    this->_acceptEmoji(emoji, boost::none, false);
}

void QJomeWindow::_emojiHoverEntered(const Emoji& emoji)
{
    this->_updateBottomLabels(&emoji);
}

void QJomeWindow::_emojiHoverLeaved(const Emoji&)
{
    this->_updateBottomLabels(_selectedEmoji);
}

void QJomeWindow::_acceptSelectedEmoji(const boost::optional<Emoji::SkinTone>& skinTone,
                                       const bool removeVs16)
{
    if (_selectedEmoji) {
        this->_acceptEmoji(*_selectedEmoji, skinTone, removeVs16);
    }
}

void QJomeWindow::_acceptEmoji(const Emoji& emoji,
                               const boost::optional<Emoji::SkinTone>& skinTone,
                               const bool removeVs16)
{
    if (skinTone && !emoji.hasSkinToneSupport()) {
        return;
    }

    emit this->emojiChosen(emoji, skinTone, removeVs16);
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
    this->_updateKwLabel(emoji);
}

namespace {

QString spanInfoLabelText(const std::string& text, const char * const hexColor,
                          const char * const addStyle = "")
{
    return qFmtFormat("<span style=\"color: #{};{}\">{}</span>", hexColor, addStyle, text);
}

QString normInfoLabelText(const std::string& text, const char * const addStyle = "")
{
    return spanInfoLabelText(text, "707070", addStyle);
}

} // namespace

void QJomeWindow::_updateInfoLabel(const Emoji * const emoji)
{
    QString text;

    if (emoji) {
        text = qFmtFormat("<b>{}</b> ", emoji->name()) +
               normInfoLabelText("(") +
               call([emoji] {
                   QStringList lst;

                   for (const auto codepoint : emoji->codepoints()) {
                       static constexpr auto italicCss = "font-style: italic;";

                           if (codepoint == 0x200d) {
                               lst.append(normInfoLabelText("ZWJ", italicCss));
                           } else if (codepoint == 0xfe0f) {
                               lst.append(normInfoLabelText("VS-16", italicCss));
                           } else {
                               lst.append(spanInfoLabelText(fmt::format("U+{:X}", codepoint), "a0a0a0"));
                           }
                       }

                   return lst;
               }).join(normInfoLabelText(", ")) +
               normInfoLabelText(")");
    }

    _wInfoLabel->setText(text);
}

void QJomeWindow::_updateVersionLabel(const Emoji * const emoji)
{
    _wVersionLabel->setText(call([emoji] {
        QString text;

        if (emoji) {
            text += QString {"Emoji <b>"} +
                    call([emoji] {
                        switch (emoji->version()) {
                        case EmojiVersion::V_0_6:
                            return "0.6&nbsp;";

                        case EmojiVersion::V_0_7:
                            return "0.7&nbsp;";

                        case EmojiVersion::V_1_0:
                            return "1.0&nbsp;";

                        case EmojiVersion::V_2_0:
                            return "2.0&nbsp;";

                        case EmojiVersion::V_3_0:
                            return "3.0&nbsp;";

                        case EmojiVersion::V_4_0:
                            return "4.0&nbsp;";

                        case EmojiVersion::V_5_0:
                            return "5.0&nbsp;";

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

                        case EmojiVersion::V_15_0:
                            return "15.0";

                        case EmojiVersion::V_15_1:
                            return "15.1";

                        default:
                            std::abort();
                        }
                    }) +
                    "</b>&nbsp;(<i>" +
                    call([emoji] {
                        switch (emoji->version()) {
                        case EmojiVersion::V_0_6:
                            return "Oct 2010";

                        case EmojiVersion::V_0_7:
                            return "Jun 2014";

                        case EmojiVersion::V_1_0:
                            return "Aug 2015";

                        case EmojiVersion::V_2_0:
                            return "Nov 2015";

                        case EmojiVersion::V_3_0:
                            return "Jun 2016";

                        case EmojiVersion::V_4_0:
                            return "Nov 2016";

                        case EmojiVersion::V_5_0:
                            return "May 2017";

                        case EmojiVersion::V_11_0:
                            return "Jun 2018";

                        case EmojiVersion::V_12_0:
                            return "Mar 2019";

                        case EmojiVersion::V_12_1:
                            return "Oct 2019";

                        case EmojiVersion::V_13_0:
                            return "Mar 2020";

                        case EmojiVersion::V_13_1:
                            return "Sep 2020";

                        case EmojiVersion::V_14_0:
                            return "Sep 2021";

                        case EmojiVersion::V_15_0:
                            return "Sep 2022";

                        case EmojiVersion::V_15_1:
                            return "Sep 2023";

                        default:
                            std::abort();
                        }
                    }) +
                    "</i>)";
        }

        return text;
    }));
}

void QJomeWindow::_updateKwLabel(const Emoji * const emoji)
{

    QString text;

    if (emoji) {
        QStringList kws;

        for (auto& kw : emoji->keywords()) {
            kws.append(QString::fromStdString(kw));
        }

        kws.sort();
        text = kws.join(normInfoLabelText(", "));
    }

    _wKwLabel->setText(text);
}

void QJomeWindow::emojiDbChanged()
{
    _wEmojis->rebuild();
    _wEmojis->showAllEmojis();
}

} // namespace jome
