/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_SETTINGS_HPP
#define _JOME_SETTINGS_HPP

#include <QSettings>

#include "emoji-db.hpp"

namespace jome {

/*
 * Updates the recent emojis of `db` from the settings.
 */
void updateRecentEmojisFromSettings(EmojiDb& db);

/*
 * Updates the settings from the recent emojis of `db`.
 */
void updateSettings(const EmojiDb& db);

} // namespace jome

#endif // _JOME_SETTINGS_HPP
