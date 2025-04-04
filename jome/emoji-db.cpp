/*
 * Copyright (C) 2019-2025 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <fstream>
#include <cstdlib>
#include <cassert>
#include <QString>
#include <QVector>
#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "emoji-db.hpp"

namespace jome {

Emoji::Emoji(std::string str, std::string name,
             std::unordered_set<std::string>&& keywords, const bool hasSkinToneSupport,
             const EmojiVersion version) :
    _str {std::move(str)},
    _name {std::move(name)},
    _keywords {std::move(keywords)},
    _hasSkinToneSupport {hasSkinToneSupport},
    _version {version}
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

std::string Emoji::strWithSkinTone(const SkinTone skinTone) const
{
    assert(_hasSkinToneSupport);
    return QString::fromUcs4(this->codepointsWithSkinTone(skinTone).data()).toStdString();
}

Emoji::Codepoints Emoji::codepointsWithSkinTone(const SkinTone skinTone) const
{
    assert(_hasSkinToneSupport);

    auto codepoints = this->codepoints();

    codepoints.insert(codepoints.begin() + 1, call([skinTone] {
        switch (skinTone) {
        case SkinTone::Light:
            return 0x1f3fb;

        case SkinTone::MediumLight:
            return 0x1f3fc;

        case SkinTone::Medium:
            return 0x1f3fd;

        case SkinTone::MediumDark:
            return 0x1f3fe;

        case SkinTone::Dark:
            return 0x1f3ff;

        default:
            std::abort();
        }
    }));

    return codepoints;
}

Emoji::Codepoints Emoji::codepoints() const
{
    Codepoints codepoints;

    for (const auto qcp : QString::fromStdString(_str).toUcs4()) {
        codepoints.push_back(qcp);
    }

    return codepoints;
}

EmojiCat::EmojiCat(std::string id, std::string name,
                   std::vector<const Emoji *>&& emojis) :
    _id {std::move(id)},
    _name {std::move(name)},
    _emojis {std::move(emojis)}
{
}

EmojiCat::EmojiCat(std::string id, std::string name) :
    _id {std::move(id)},
    _name {std::move(name)}
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
    _emojisPngPath {dir + '/' + "emojis-" + std::to_string(this->emojiSizeInt()) + ".png"}
{
    this->_createEmojis(dir);
    this->_createCats(dir);
    this->_createEmojiPngLocations(dir);
}

nlohmann::json EmojiDb::_loadJson(const std::string& dir, const std::string& file)
{
    std::ifstream f {dir + '/' + file};
    nlohmann::json json;

    f >> json;
    return json;
}

void EmojiDb::_createEmojis(const std::string& dir)
{
    const auto jsonEmojis = this->_loadJson(dir, "emojis.json");

    for (auto& emojiKeyJsonValPair : jsonEmojis.items()) {
        auto emoji = call([&emojiKeyJsonValPair] {
            auto& jsonVal = emojiKeyJsonValPair.value();

            return std::make_unique<const Emoji>(emojiKeyJsonValPair.key(),
                                                 jsonVal.at("name"),
                                                 call([&jsonVal] {
                                                     std::unordered_set<std::string> keywords;

                                                     for (auto& kw : jsonVal.at("keywords")) {
                                                        keywords.insert(kw);
                                                     }

                                                     return keywords;
                                                 }), jsonVal.at("has-skin-tone-support"),
                                                 call([&jsonVal] {
                                                     const auto str = jsonVal.at("version").get<std::string>();

                                                     if (str == "0.6") {
                                                         return EmojiVersion::V_0_6;
                                                     } else if (str == "0.7") {
                                                         return EmojiVersion::V_0_7;
                                                     } else if (str == "1.0") {
                                                         return EmojiVersion::V_1_0;
                                                     } else if (str == "2.0") {
                                                         return EmojiVersion::V_2_0;
                                                     } else if (str == "3.0") {
                                                         return EmojiVersion::V_3_0;
                                                     } else if (str == "4.0") {
                                                         return EmojiVersion::V_4_0;
                                                     } else if (str == "5.0") {
                                                         return EmojiVersion::V_5_0;
                                                     } else if (str == "11.0") {
                                                         return EmojiVersion::V_11_0;
                                                     } else if (str == "12.0") {
                                                         return EmojiVersion::V_12_0;
                                                     } else if (str == "12.1") {
                                                         return EmojiVersion::V_12_1;
                                                     } else if (str == "13.0") {
                                                         return EmojiVersion::V_13_0;
                                                     } else if (str == "13.1") {
                                                         return EmojiVersion::V_13_1;
                                                     } else {
                                                         assert(str == "14.0");
                                                         return EmojiVersion::V_14_0;
                                                     }
                                                 }));
        });

        for (auto& keyword : emoji->keywords()) {
            _keywords.insert(keyword);

            auto it = _keywordEmojis.find(keyword);

            if (it == _keywordEmojis.end()) {
                it = _keywordEmojis.insert(std::make_pair(keyword,
                                                          decltype(_keywordEmojis)::mapped_type {})).first;
            }

            it->second.insert(emoji.get());
        }

        _emojis[emoji->str()] = std::move(emoji);
    }
}

void EmojiDb::_createCats(const std::string& dir)
{
    // first, special category: recent emojis
    _cats.push_back(std::make_unique<EmojiCat>("recent", "Recent"));
    _recentEmojisCat = _cats.back().get();

    const auto jsonCats = this->_loadJson(dir, "cats.json");

    for (auto& jsonCat : jsonCats) {
        _cats.push_back(call([this, &jsonCat] {
            return std::make_unique<EmojiCat>(jsonCat.at("id"), jsonCat.at("name"),
                                              call([this, &jsonCat] {
                                                  std::vector<const Emoji *> emojis;

                                                  for (auto& jsonEmoji : jsonCat.at("emojis")) {
                                                      emojis.push_back(_emojis[jsonEmoji].get());
                                                  }

                                                  return emojis;
                                              }));
        }));
    }
}

void EmojiDb::_createEmojiPngLocations(const std::string& dir)
{
    const std::string fileName {
        "emojis-png-locations-" + std::to_string(this->emojiSizeInt()) + ".json"
    };
    const auto pngLocationsJson = this->_loadJson(dir, fileName);

    for (auto& keyJsonValPair : pngLocationsJson.items()) {
        auto& jsonLoc = keyJsonValPair.value();

        _emojiPngLocations[_emojis[keyJsonValPair.key()].get()] = {
            static_cast<unsigned int>(jsonLoc.at(0)),
            static_cast<unsigned int>(jsonLoc.at(1))
        };
    }
}

void EmojiDb::findEmojis(const std::string& cat, const std::string& needlesStr,
                         std::vector<const Emoji *>& results) const
{
    // split needles string into individual needles
    _tmpNeedles.clear();
    boost::split(_tmpNeedles, needlesStr, boost::is_any_of(" "));

    if (_tmpNeedles.empty()) {
        // nothing to search
        return;
    }

    // trim category
    const auto catTrimmed = call([&cat] {
        std::string catTrimmed {cat};

        boost::trim(catTrimmed);
        return catTrimmed;
    });

    // this is to avoid duplicate entries in `results`
    _tmpFoundEmojis.clear();

    for (auto& cat : _cats) {
        if (!catTrimmed.empty() && cat->lcName().find(catTrimmed) == std::string::npos) {
            // we don't want to search this category
            continue;
        }

        for (auto& emoji : cat->emojis()) {
            auto select = true;

            for (auto& keyword : emoji->keywords()) {
                select = true;

                for (auto& needle : _tmpNeedles) {
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

            if (_tmpFoundEmojis.find(emoji) != _tmpFoundEmojis.end()) {
                // we already have it: next emoji
                continue;
            }

            results.push_back(emoji);
            _tmpFoundEmojis.insert(emoji);
        }
    }
}

void EmojiDb::recentEmojis(std::vector<const Emoji *>&& emojis)
{
    assert(_recentEmojisCat);
    _recentEmojisCat->emojis() = std::move(emojis);
}

void EmojiDb::addRecentEmoji(const Emoji& emoji)
{
    assert(_recentEmojisCat);

    auto& emojis = _recentEmojisCat->emojis();

    while (true) {
        auto existingIt = std::find(emojis.begin(), emojis.end(), &emoji);

        if (existingIt == emojis.end()) {
            break;
        }

        emojis.erase(existingIt);
    }

    emojis.insert(emojis.begin(), &emoji);

    static constexpr auto maxRecentEmojis = 30U;

    if (emojis.size() > maxRecentEmojis) {
        emojis.resize(maxRecentEmojis);
    }
}

} // namespace jome
