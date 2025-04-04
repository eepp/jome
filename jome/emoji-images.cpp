/*
 * Copyright (C) 2019-2025 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "emoji-images.hpp"

namespace jome {

EmojiImages::EmojiImages(const EmojiDb& db)
{
    this->_createPixmaps(db);
}

void EmojiImages::_createPixmaps(const EmojiDb& db)
{
    auto emojisPixmap = QPixmap::fromImage(QImage {QString::fromStdString(db.emojisPngPath())});

    for (auto& emojiPngLocation : db.emojiPngLocations()) {
        auto& pngLoc = emojiPngLocation.second;
        const auto emojiSize = db.emojiSizeInt();

        _emojiPixmaps[emojiPngLocation.first] = std::make_unique<QPixmap>(emojisPixmap.copy(pngLoc.x,
                                                                                            pngLoc.y,
                                                                                            emojiSize,
                                                                                            emojiSize));
    }
}

} // namespace jome
