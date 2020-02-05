/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <fstream>
#include <cstdlib>
#include <cassert>
#include <boost/algorithm/string.hpp>

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

const std::string& Emoji::lcName() const
{
    if (_lcName.empty()) {
        _lcName = _name;
        boost::algorithm::to_lower(_lcName);
    }

    return _lcName;
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

    auto codepoints = this->codepoints();

    if (skinTone == SkinTone::NONE) {
        return codepoints;
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

    codepoints.insert(std::begin(codepoints) + 1, skinToneCodepoint);
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

EmojiCat::EmojiCat(const std::string& id, const std::string& name) :
    _id {id},
    _name {name}
{
}

const std::string& EmojiCat::lcName() const
{
    if (_lcName.empty()) {
        _lcName = _name;
        boost::algorithm::to_lower(_lcName);
    }

    return _lcName;
}

EmojiDb::EmojiDb(const std::string& dir, const EmojiSize emojiSize) :
    _emojiSize {emojiSize},
    _emojisPngPath {
        dir + '/' + "emojis-" +
        std::to_string(this->emojiSizeInt()) + ".png"
    }
{
    this->_createEmojis(dir);
    this->_createCats(dir);
    this->_setRecentEmojisCatFromSettings();
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
    // first, special category: recent emojis
    _cats.push_back(std::make_unique<EmojiCat>("recent", "Recent"));
    _recentEmojisCat = _cats.back().get();

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

        auto cat = std::make_unique<EmojiCat>(idJson.ToString(),
                                              nameJson.ToString(),
                                              std::move(emojis));

        _cats.push_back(std::move(cat));
    }
}

void EmojiDb::_createEmojiPngLocations(const std::string& dir)
{
    const std::string fileName {
        "emojis-png-locations-" +
        std::to_string(this->emojiSizeInt()) + ".json"
    };
    const auto pngLocationsJson = this->_loadJson(dir, fileName);

    for (const auto& keyValPair : pngLocationsJson.ObjectRange()) {
        const auto& emojiStr = keyValPair.first;
        const auto& valJson = keyValPair.second;

        _emojiPngLocations[_emojis[emojiStr].get()] = {
            static_cast<unsigned int>(valJson.at(0).ToInt()),
            static_cast<unsigned int>(valJson.at(1).ToInt())
        };
    }
}

void EmojiDb::findEmojis(const std::string& cat, const std::string& needlesStr,
                         std::vector<const Emoji *>& results) const
{
    std::string catTrimmed {cat};

    // split needles string into individual needles
    _tmpNeedles.clear();
    boost::split(_tmpNeedles, needlesStr, boost::is_any_of(" "));

    if (_tmpNeedles.empty()) {
        // nothing to search
        return;
    }

    // trim category
    boost::trim(catTrimmed);

    // this is to avoid duplicate entries in `results`
    _tmpFoundEmojis.clear();

    for (const auto& cat : _cats) {
        if (!catTrimmed.empty() && cat->lcName().find(catTrimmed) == std::string::npos) {
            // we don't want to search this category
            continue;
        }

        for (const auto& emoji : cat->emojis()) {
            bool select = true;

            for (const auto& keyword : emoji->keywords()) {
                select = true;

                for (const auto& needle : _tmpNeedles) {
                    if (needle.empty()) {
                        continue;
                    }

                    if (keyword.find(needle) == std::string::npos) {
                        // this keyword does not this needle
                        select = false;
                        break;
                    }
                }

                if (select) {
                    break;
                }
            }

            if (!select) {
                // not selected: next emoji
                continue;
            }

            if (_tmpFoundEmojis.find(emoji) != std::end(_tmpFoundEmojis)) {
                // we already have it: next emoji
                continue;
            }

            results.push_back(emoji);
            _tmpFoundEmojis.insert(emoji);
        }
    }
}

void EmojiDb::syncWithSettings()
{
    this->_setRecentEmojisCatFromSettings();
}

void EmojiDb::_setRecentEmojisCatFromSettings()
{
    assert(_recentEmojisCat);
    _recentEmojisCat->emojis().clear();
    _settings.sync();

    const auto recentEmojisVar = _settings.value("recent-emojis");

    if (!recentEmojisVar.canConvert<QList<QVariant>>()) {
        return;
    }

    const auto recentEmojisList = recentEmojisVar.toList();

    for (const auto& emojiStrVar : recentEmojisList) {
        if (!emojiStrVar.canConvert<QString>()) {
            continue;
        }

        const auto emojiStr = emojiStrVar.toString();
        const auto it = _emojis.find(emojiStr.toUtf8().constData());

        if (it == std::end(_emojis)) {
            continue;
        }

        _recentEmojisCat->emojis().push_back(it->second.get());
    }
}

void EmojiDb::addRecentEmoji(const Emoji& emoji)
{
    /*
     * Refresh the recent emojis category from the settings first as it
     * is possible that another jome instance updated the settings in
     * the meantime.
     *
     * Then, add the emoji to the recent emojis category, and update the
     * settings accordingly.
     */
    this->_setRecentEmojisCatFromSettings();
    assert(_recentEmojisCat);

    auto& emojis = _recentEmojisCat->emojis();

    while (true) {
        auto existingIt = std::find(std::begin(emojis), std::end(emojis),
                                    &emoji);

        if (existingIt == std::end(emojis)) {
            break;
        }

        emojis.erase(existingIt);
    }

    emojis.insert(std::begin(emojis), &emoji);

    constexpr auto maxRecentEmojis = 30U;

    if (emojis.size() > maxRecentEmojis) {
        emojis.resize(maxRecentEmojis);
    }

    QList<QVariant> emojiList;

    for (const auto emoji : _recentEmojisCat->emojis()) {
        const auto emojiStr = QString::fromStdString(emoji->str());

        emojiList.append(emojiStr);
    }

    // update and write the settings
    _settings.setValue("recent-emojis", emojiList);
    _settings.sync();
}

} // namespace jome
