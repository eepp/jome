/*
 * Copyright (C) 2019-2025 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <optional>
#include <functional>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <QString>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QtDebug>
#include <QFile>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <fmt/format.h>

#include "nlohmann/json_fwd.hpp"
#include "utils.hpp"
#include "emoji-db.hpp"

namespace jome {
namespace {

QString cpStr(const Emoji::Codepoints& codepoints)
{
    QStringList parts;

    for (const auto cp : codepoints) {
        parts.append(qFmtFormat("u+{:x}", cp));
    }

    return parts.join(' ');
}

} // namespace

Emoji::Emoji(QString str, QString name,
             std::unordered_set<QString>&& keywords,
             std::unordered_set<unsigned int>&& modBaseIndexes,
             const EmojiVersion version) :
    _str {std::move(str)},
    _name {std::move(name)},
    _lcName {_name.toLower()},
    _cpStr {cpStr(this->codepoints())},
    _keywords {std::move(keywords)},
    _modBaseIndexes {std::move(modBaseIndexes)},
    _version {version}
{
}

QString Emoji::str(const std::optional<SkinTone> skinTone,
                   const bool withVs16) const
{
    const auto codepoints = this->codepoints(skinTone, withVs16);

    return QString::fromUcs4(codepoints.data(), codepoints.size());
}

Emoji::Codepoints Emoji::codepoints(const std::optional<SkinTone> skinTone,
                                    const bool withVs16) const
{
    Codepoints codepoints;

    // skin tone modifier codepoint, if needed
    const auto skinToneCp = std::invoke([&skinTone] {
        if (!skinTone) {
            return 0;
        }

        switch (*skinTone) {
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
    });

    /*
     * Build the final codepoint sequence, optionally:
     *
     * • Removing VS-16 (U+FE0F) codepoints.
     *
     * • Inserting a skin tone modifier after each emoji modifier base
     *   codepoint (identified by its index in the original sequence).
     */
    unsigned int origIdx = 0;

    for (const auto qcp : _str.toUcs4()) {
        if (!withVs16 && qcp == 0xfe0f) {
            ++origIdx;
            continue;
        }

        codepoints.push_back(qcp);

        if (skinTone) {
            assert(!_modBaseIndexes.empty());

            if (_modBaseIndexes.count(origIdx) > 0) {
                codepoints.push_back(skinToneCp);
            }
        }

        ++origIdx;
    }

    return codepoints;
}

EmojiCat::EmojiCat(QString id, QString name, std::vector<const Emoji *>&& emojis) :
    _id {std::move(id)},
    _name {std::move(name)},
    _lcName {_name.toLower()},
    _emojis {std::move(emojis)}
{
}

EmojiCat::EmojiCat(QString id, QString name) :
    _id {std::move(id)},
    _name {std::move(name)},
    _lcName {_name.toLower()}
{
}

EmojiDb::EmojiDb(const QString& dir, const EmojiSize emojiSize,
                 const unsigned int maxRecentEmojis, const bool noRecentCat,
                 const bool incRecentInFindResults) :
    _emojiSize {emojiSize},
    _emojisPngPath {qFmtFormat("{}/emojis-{}.png", dir.toStdString(), this->emojiSizeInt())},
    _maxRecentEmojis {maxRecentEmojis},
    _incRecentInFindResults {incRecentInFindResults}
{
    this->_createEmojis(dir);
    this->_createCats(dir, noRecentCat);
    this->_createEmojiPngLocations(dir);
}

namespace {

/*
 * Returns the root JSON value of the JSON file `path`.
 */
nlohmann::json loadJson(const QString& path)
{
    std::ifstream f {path.toStdString()};
    nlohmann::json json;

    f >> json;
    return json;
}

/*
 * Returns the root JSON value of the JSON file `file` within `dir`.
 */
nlohmann::json loadJson(const QString& dir, const QString& file)
{
    return loadJson(qFmtFormat("{}/{}", dir.toStdString(), file.toStdString()));
}

/*
 * Warns that user-defined emoji keywords will be disabled because the
 * JSON file `path` is invalid.
 *
 * Print `msg` as the reason.
 */
void warnNoUserEmojiKeywords(const QString& path, const QString& msg)
{
    qWarning().noquote() << qFmtFormat("{}: {}", path.toStdString(), msg.toStdString());
    qWarning() << "jome will continue without user emoji keywords";
}

/*
 * Loads the user-defined emoji keywords, validates the JSON object,
 * and returns it or an empty object if invalid.
 */
nlohmann::json loadUserEmojisJson()
{
    const auto path = qFmtFormat("{}/{}",
                                 QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first().toStdString(),
                                 "jome/emojis.json");

    if (!QFile::exists(path)) {
        // this is an optional file
        return nlohmann::json::object();
    }

    auto jsonUserEmojis = std::invoke([&path]() -> std::optional<nlohmann::json> {
        try {
            return loadJson(path);
        } catch (const nlohmann::json::exception& exc) {
            warnNoUserEmojiKeywords(path, qFmtFormat("failed to load JSON file: {}", exc.what()));
            return std::nullopt;
        }
    });

    if (!jsonUserEmojis) {
        return nlohmann::json::object();
    }

    // validate
    if (!jsonUserEmojis->is_object()) {
        warnNoUserEmojiKeywords(path, "expecting a root JSON object");
        return nlohmann::json::object();
    }

    for (const auto& keyJsonValPair : jsonUserEmojis->items()) {
        if (!keyJsonValPair.value().is_object()) {
            warnNoUserEmojiKeywords(path, qFmtFormat("emoji `{}`: expecting an object",
                                                     keyJsonValPair.key()));
            return nlohmann::json::object();
        }

        const auto validateKeywords = [&keyJsonValPair, &path](const std::string& key) {
            const auto it = keyJsonValPair.value().find(key);

            if (it == keyJsonValPair.value().end()) {
                return true;
            }

            if (!it->is_array()) {
                warnNoUserEmojiKeywords(path, qFmtFormat("emoji `{}`: `{}`: expecting an array",
                                                         keyJsonValPair.key(), key));
                return false;
            }

            for (const auto& jsonKeyword : *it) {
                if (!jsonKeyword.is_string()) {
                    warnNoUserEmojiKeywords(path, qFmtFormat("emoji `{}`: `{}`: expecting an array of strings",
                                                             keyJsonValPair.key(), key));
                    return false;
                }
            }

            return true;
        };

        if (!validateKeywords("keywords")) {
            return nlohmann::json::object();
        }

        if (!validateKeywords("extra-keywords")) {
            return nlohmann::json::object();
        }
    }

    return *jsonUserEmojis;
}

/*
 * Returns a set of strings from the JSON array of JSON
 * strings `jsonArray`.
 */
std::unordered_set<QString> qStrSetFromJsonStrArray(const nlohmann::json& jsonArray)
{
    assert(jsonArray.is_array());

    std::unordered_set<QString> set;

    for (auto& jsonStr : jsonArray) {
        assert(jsonStr.is_string());
        set.insert(QString::fromStdString(jsonStr));
    }

    return set;
}

/*
 * Returns the set of effective emoji keywords of the emoji having the
 * string `emojiStr` given:
 *
 * • The built-in keywords `jsonKeywords`.
 *
 * • The user-defined emoji keywords `jsonUserEmojis` (the
 *   whole object).
 */
std::unordered_set<QString> effectiveEmojiKeywords(const QString& emojiStr,
                                                   const nlohmann::json& jsonKeywords,
                                                   const nlohmann::json& jsonUserEmojis)
{
    auto jsonUserKeywords = nlohmann::json::array();
    auto jsonUserExtraKeywords = nlohmann::json::array();

    if (const auto it = jsonUserEmojis.find(emojiStr.toStdString()); it != jsonUserEmojis.end()) {
        if (const auto jsonKeywordsIt = it->find("keywords"); jsonKeywordsIt != it->end()) {
            jsonUserKeywords = *jsonKeywordsIt;
        }

        if (const auto jsonKeywordsIt = it->find("extra-keywords"); jsonKeywordsIt != it->end()) {
            jsonUserExtraKeywords = *jsonKeywordsIt;
        }
    }

    std::unordered_set<QString> keywords;

    if (jsonUserKeywords.empty()) {
        // start with default keywords
        keywords = qStrSetFromJsonStrArray(jsonKeywords);
    } else {
        // start with user keywords
        keywords = qStrSetFromJsonStrArray(jsonUserKeywords);
    }

    {
        const auto userExtraKeywords = qStrSetFromJsonStrArray(jsonUserExtraKeywords);

        keywords.insert(userExtraKeywords.begin(), userExtraKeywords.end());
    }

    return keywords;
}

} // namespace

void EmojiDb::_createEmojis(const QString& dir)
{
    // load jome's emoji database
    const auto jsonEmojis = loadJson(dir, "emojis.json");

    // load user-defined emoji keywords
    const auto jsonUserEmojis = loadUserEmojisJson();

    // build each emoji object
    for (auto& emojiKeyJsonValPair : jsonEmojis.items()) {
        auto emoji = std::invoke([&emojiKeyJsonValPair, &jsonUserEmojis] {
            const auto emojiStr = QString::fromStdString(emojiKeyJsonValPair.key());
            auto& jsonVal = emojiKeyJsonValPair.value();

            return std::make_unique<const Emoji>(emojiStr,
                                                 QString::fromStdString(jsonVal.at("name")),
                                                 effectiveEmojiKeywords(emojiStr,
                                                                        jsonVal.at("keywords"),
                                                                        jsonUserEmojis),
                                                 std::invoke([&jsonVal] {
                                                     std::unordered_set<unsigned int> indexes;

                                                     if (const auto it = jsonVal.find("mod-base-indexes"); it != jsonVal.end()) {
                                                         for (const auto& idx : *it) {
                                                             indexes.insert(idx.get<unsigned int>());
                                                         }
                                                     }

                                                     return indexes;
                                                 }),
                                                 std::invoke([&jsonVal] {
                                                     if (const auto str = jsonVal.at("version").get<std::string>(); str == "0.6") {
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
                                                     } else if (str == "14.0") {
                                                         return EmojiVersion::V_14_0;
                                                     } else if (str == "15.0") {
                                                         return EmojiVersion::V_15_0;
                                                     } else if (str == "15.1") {
                                                         return EmojiVersion::V_15_1;
                                                     } else if (str == "16.0") {
                                                         return EmojiVersion::V_16_0;
                                                     } else {
                                                         assert(str == "17.0");
                                                         return EmojiVersion::V_17_0;
                                                     }
                                                 }));
        });

        _emojis[emoji->str()] = std::move(emoji);
    }
}

void EmojiDb::_createCats(const QString& dir, const bool noRecentCat)
{
    if (!noRecentCat) {
        // first, special category: recent emojis
        _cats.push_back(std::make_unique<EmojiCat>("recent", "Recent"));
        _recentEmojisCat = _cats.back().get();
    }

    // load jome's category database
    const auto jsonCats = loadJson(dir, "cats.json");

    // build each category
    for (auto& jsonCat : jsonCats) {
        _cats.push_back(std::invoke([this, &jsonCat] {
            return std::make_unique<EmojiCat>(QString::fromStdString(jsonCat.at("id")),
                                              QString::fromStdString(jsonCat.at("name")),
                                              std::invoke([this, &jsonCat] {
                                                  std::vector<const Emoji *> emojis;

                                                  for (auto& jsonEmoji : jsonCat.at("emojis")) {
                                                      emojis.push_back(_emojis[QString::fromStdString(jsonEmoji)].get());
                                                  }

                                                  return emojis;
                                              }));
        }));
    }
}

void EmojiDb::_createEmojiPngLocations(const QString& dir)
{
    // load jome's PNG locations
    const auto pngLocationsJson = loadJson(dir,
                                           qFmtFormat("emojis-png-locations-{}.json", this->emojiSizeInt()));

    // assign each emoji to its PNG location
    for (auto& [key, jsonLoc] : pngLocationsJson.items()) {
        _emojiPngLocations[_emojis[QString::fromStdString(key)].get()] = {
            static_cast<unsigned int>(jsonLoc.at(0)),
            static_cast<unsigned int>(jsonLoc.at(1))
        };
    }
}

void EmojiDb::findEmojis(QString catName, const QString& needlesStr,
                         std::vector<const Emoji *>& results) const
{
    // split `needlesStr` into individual needles
    const auto needles = needlesStr.toLower().split(QRegularExpression {" +"}, Qt::SkipEmptyParts);

    // trim category
    catName = catName.trimmed();

    // clear temporary results
    _tmpFindResults.clear();
    _tmpFindResultEmojis.clear();

    // handle specific codepoint search
    if (needles.size() == 1 && needles.first().size() >= 3 && needles.first().startsWith("u+")) {
        for (auto& cat : _cats) {
            if (!catName.isEmpty() && cat->isRecent()) {
                // invalid
                continue;
            }

            if (cat->isRecent()) {
                if (!_incRecentInFindResults) {
                    // exclude "Recent" category
                    continue;
                }
            } else if (!catName.isEmpty() && !cat->lcName().contains(catName)) {
                // we don't even want to search this category
                continue;
            }

            for (auto emoji : cat->emojis()) {
                if (emoji->codepointStr().contains(needles.first()) &&
                        _tmpFindResultEmojis.count(emoji) == 0) {
                    results.push_back(emoji);
                    _tmpFindResultEmojis.insert(emoji);
                }
            }
        }

        return;
    }

    auto pos = 0U;

    for (auto& cat : _cats) {
        auto initScore = 0U;

        if (!catName.isEmpty() && cat->isRecent()) {
            // invalid
            continue;
        }

        if (cat->isRecent()) {
            if (_incRecentInFindResults) {
                // boost to get them before the other categories
                initScore = 1000;
            } else {
                // exclude "Recent" category
                continue;
            }
        } else if (!catName.isEmpty() && !cat->lcName().contains(catName)) {
            // we don't even want to search this category
            continue;
        }

        for (auto emoji : cat->emojis()) {
            auto score = initScore;

            for (auto& needle : needles) {
                auto needleScore = 0U;

                if (emoji->lcName() == needle) {
                    needleScore = 100;
                } else if (emoji->lcName().startsWith(needle)) {
                    needleScore = 80;
                } else if (emoji->lcName().contains(needle)) {
                    needleScore = 60;
                }

                auto addToNeedleScore = 0U;

                for (auto& keyword : emoji->keywords()) {
                    if (keyword == needle) {
                        addToNeedleScore = 40;
                        break;
                    }

                    if (keyword.startsWith(needle)) {
                        addToNeedleScore = std::max(addToNeedleScore, 30U);
                    } else if (keyword.contains(needle)) {
                        addToNeedleScore = std::max(addToNeedleScore, 20U);
                    }
                }

                needleScore += addToNeedleScore;

                if (needleScore == 0) {
                    score = 0;
                    break;
                }

                score += needleScore;
            }

            if (needles.isEmpty() || score > 0) {
                if (_tmpFindResultEmojis.count(emoji) == 0) {
                    _tmpFindResults.insert({score, pos, emoji});
                    _tmpFindResultEmojis.insert(emoji);
                }
            }

            ++pos;
        }
    }

    for (auto it = _tmpFindResults.crbegin(); it != _tmpFindResults.crend(); ++it) {
        results.push_back(it->emoji);
    }
}

void EmojiDb::recentEmojis(std::vector<const Emoji *>&& emojis)
{
    if (!_recentEmojisCat) {
        // no "Recent" category: return
        return;
    }

    _recentEmojisCat->emojis() = std::move(emojis);

    if (_recentEmojisCat->emojis().size() > _maxRecentEmojis) {
        // clip
        _recentEmojisCat->emojis().resize(_maxRecentEmojis);
    }
}

void EmojiDb::addRecentEmoji(const Emoji& emoji)
{
    if (!_recentEmojisCat) {
        // no "Recent" category: return
        return;
    }

    auto& emojis = _recentEmojisCat->emojis();

    // remove from current list
    while (true) {
        auto existingIt = std::find(emojis.begin(), emojis.end(), &emoji);

        if (existingIt == emojis.end()) {
            break;
        }

        emojis.erase(existingIt);
    }

    // insert at the beginning
    emojis.insert(emojis.begin(), &emoji);

    if (emojis.size() > _maxRecentEmojis) {
        // clip
        emojis.resize(_maxRecentEmojis);
    }
}

} // namespace jome
