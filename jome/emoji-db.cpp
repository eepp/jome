/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <fstream>
#include <cstdlib>
#include <cassert>

#include "emoji-db.hpp"
#include "simple-json.hpp"
#include "tinyutf8.hpp"

namespace jome {

Emoji::Emoji(const std::string& str, const std::string& name,
             std::unordered_set<std::string>&& keywords,
             const bool hasSkinToneSupport) :
    _str {str},
    _name {name},
    _keywords {std::move(keywords)},
    _hasSkinToneSupport {hasSkinToneSupport}
{
}

std::string Emoji::strWithSkinTone(SkinTone skinTone) const
{
    assert(_hasSkinToneSupport);

    const auto codepoints = this->codepoints();

    return utf8_string {std::begin(codepoints), std::end(codepoints)}.c_str();
}

Emoji::Codepoints Emoji::codepointsWithSkinTone(SkinTone skinTone) const
{
    assert(_hasSkinToneSupport);

    const auto origCodepoints = this->codepoints();

    if (skinTone == SkinTone::NONE) {
        return origCodepoints;
    }

    Codepoint skinToneCodepoint = 0;

    switch (skinTone) {
    case SkinTone::LIGHT:
        skinToneCodepoint = 0x1f3fb;
        break;

    case SkinTone::MEDIUM_LIGHT:
        skinToneCodepoint = 0x1f3fc;
        break;

    case SkinTone::MEDIUM:
        skinToneCodepoint = 0x1f3fd;
        break;

    case SkinTone::MEDIUM_DARK:
        skinToneCodepoint = 0x1f3fe;
        break;

    case SkinTone::DARK:
        skinToneCodepoint = 0x1f3ff;
        break;

    default:
        std::abort();
    }

    Codepoints codepoints;

    for (const auto codepoint : origCodepoints) {
        codepoints.push_back(codepoint);

        if (codepoint >= 0x1f000) {
            codepoints.push_back(skinToneCodepoint);
        }
    }

    return codepoints;
}

Emoji::Codepoints Emoji::codepoints() const
{
    Codepoints codepoints;
    const utf8_string utf8Str {_str};

    std::copy(std::begin(utf8Str), std::end(utf8Str),
              std::back_inserter(codepoints));
    return codepoints;
}

EmojiCat::EmojiCat(const std::string& id, const std::string& name,
                   std::vector<const Emoji *>&& emojis) :
    _id {id},
    _name {name},
    _emojis {std::move(emojis)}
{
}

EmojiDb::EmojiDb(const std::string& dir) :
    _emojisPngPath {dir + '/' + "emojis.png"}
{
    this->_createEmojis(dir);
    this->_createCats(dir);
    this->_createEmojiPngLocations(dir);
}

json::JSON EmojiDb::_loadJson(const std::string& dir, const std::string& file)
{
    std::ifstream f {dir + '/' + file};
    std::string str {std::istreambuf_iterator<char> {f},
                     std::istreambuf_iterator<char> {}};
    return json::JSON::Load(str);
}

void EmojiDb::_createEmojis(const std::string& dir)
{
    const auto emojisJson = this->_loadJson(dir, "emojis.json");

    for (const auto& keyValPair : emojisJson.ObjectRange()) {
        const auto& emojiStr = keyValPair.first;
        const auto& valJson = keyValPair.second;
        const auto& nameJson = valJson.at("name");
        const auto& hasSkinToneSupport = valJson.at("has-skin-tone-support").ToBool();
        const auto& keywordsJson = valJson.at("keywords");
        std::unordered_set<std::string> keywords;

        for (const auto& kw : keywordsJson.ArrayRange()) {
            keywords.insert(kw.ToString());
        }

        auto emoji = std::make_unique<const Emoji>(emojiStr,
                                                   nameJson.ToString(),
                                                   std::move(keywords),
                                                   hasSkinToneSupport);

        for (const auto& keyword : emoji->keywords()) {
            _keywords.insert(keyword);

            auto it = _keywordEmojis.find(keyword);

            if (it == std::end(_keywordEmojis)) {
                it = _keywordEmojis.insert(std::make_pair(keyword,
                                                          decltype(_keywordEmojis)::mapped_type {})).first;
            }

            it->second.insert(emoji.get());
        }

        _emojis[emojiStr] = std::move(emoji);
    }
}

void EmojiDb::_createCats(const std::string& dir)
{
    const auto catsJson = this->_loadJson(dir, "cats.json");

    for (const auto& catJson : catsJson.ArrayRange()) {
        const auto& idJson = catJson.at("id");
        const auto& nameJson = catJson.at("name");
        const auto& emojisJson = catJson.at("emojis");
        std::vector<const Emoji *> emojis;

        for (const auto& emojiJson : emojisJson.ArrayRange()) {
            const auto emojiStr = emojiJson.ToString();
            emojis.push_back(_emojis[emojiStr].get());
        }

        auto cat = std::make_unique<const EmojiCat>(idJson.ToString(),
                                                    nameJson.ToString(),
                                                    std::move(emojis));

        _cats.push_back(std::move(cat));
    }
}

void EmojiDb::_createEmojiPngLocations(const std::string& dir)
{
    const auto pngLocationsJson = this->_loadJson(dir,
                                                  "emojis-png-locations.json");

    for (const auto& keyValPair : pngLocationsJson.ObjectRange()) {
        const auto& emojiStr = keyValPair.first;
        const auto& valJson = keyValPair.second;

        _emojiPngLocations[_emojis[emojiStr].get()] = {
            static_cast<unsigned int>(valJson.at(0).ToInt()),
            static_cast<unsigned int>(valJson.at(1).ToInt())
        };
    }
}

} // namespace jome
