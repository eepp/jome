/*
 * Copyright (C) 2021 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <QDesktopServices>
#include <QString>
#include <QUrl>

#include "emojipedia.hpp"
#include "emoji-db.hpp"

namespace jome {

void gotoEmojipediaPage(const Emoji& emoji)
{
    QDesktopServices::openUrl(QUrl {
        QString {"https://emojipedia.org/search?q="} + QString::fromStdString(emoji.str())
    });
}

} // namespace jome
