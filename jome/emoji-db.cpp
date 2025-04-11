/*
 * Copyright (C) 2019-2025 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <boost/optional/optional.hpp>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <QString>
#include <QVector>
#include <QStandardPaths>
#include <QtDebug>
#include <QFile>
#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <qglobal.h>
#include <qstandardpaths.h>

#include "nlohmann/json_fwd.hpp"
#include "utils.hpp"
#include "emoji-db.hpp"

namespace jome {

Emoji::Emoji(QString str, QString name,
             std::unordered_set<QString>&& keywords, const bool hasSkinToneSupport,
             const EmojiVersion version) :
    _str {std::move(str)},
    _name {std::move(name)},
    _keywords {std::move(keywords)},
    _hasSkinToneSupport {hasSkinToneSupport},
    _version {version}
{
}

QString Emoji::str(const boost::optional<SkinTone>& skinTone,
                   const bool withVs16) const
{
    const auto codepoints = this->codepoints(skinTone, withVs16);

    return QString::fromUcs4(codepoints.data(), codepoints.size());
}

Emoji::Codepoints Emoji::codepoints(const boost::optional<SkinTone>& skinTone,
                                    const bool withVs16) const
{
    Codepoints codepoints;

    for (const auto qcp : _str.toUcs4()) {
        if (!withVs16 && qcp == 0xfe0f) {
            continue;
        }

        codepoints.push_back(qcp);
    }

    if (skinTone) {
        assert(_hasSkinToneSupport);

        codepoints.insert(codepoints.begin() + 1, call([&skinTone] {
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
        }));
    }

    return codepoints;
}

EmojiCat::EmojiCat(QString id, QString name, std::vector<const Emoji *>&& emojis) :
    _id {std::move(id)},
    _name {std::move(name)},
    _emojis {std::move(emojis)}
{
}

EmojiCat::EmojiCat(QString id, QString name) :
    _id {std::move(id)},
    _name {std::move(name)}
{
}

EmojiDb::EmojiDb(const QString& dir, const EmojiSize emojiSize,
                 const unsigned int maxRecentEmojis, const bool noRecentCat) :
    _emojiSize {emojiSize},
    _emojisPngPath {qFmtFormat("{}/emojis-{}.png", dir.toStdString(), this->emojiSizeInt())},
    _maxRecentEmojis {maxRecentEmojis}
{
    this->_createEmojis(dir);
    this->_createCats(dir, noRecentCat);
    this->_createEmojiPngLocations(dir);
}

namespace {

nlohmann::json loadJson(const QString& path)
{
    std::ifstream f {path.toStdString()};
    nlohmann::json json;

    f >> json;
    return json;
}

nlohmann::json loadJson(const QString& dir, const QString& file)
{
    return loadJson(qFmtFormat("{}/{}", dir.toStdString(), file.toStdString()));
}

void warnNoUserEmojiKeywords(const QString& path, const QString& msg)
{
    qWarning().noquote() << qFmtFormat("{}: {}", path.toStdString(), msg.toStdString());
    qWarning() << "jome will continue without user emoji keywords";
}

nlohmann::json loadUserEmojisJson()
{
    const auto path = qFmtFormat("{}/{}",
                                 QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first().toStdString(),
                                 "jome/emojis.json");

    if (!QFile::exists(path)) {
        // this is an optional file
        return nlohmann::json::object();
    }

    auto jsonUserEmojis = call([&path]() -> boost::optional<nlohmann::json> {
        try {
            return loadJson(path);
        } catch (const nlohmann::json::exception& exc) {
            warnNoUserEmojiKeywords(path, qFmtFormat("failed to load JSON file: {}", exc.what()));
            return boost::none;
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

std::unordered_set<QString> effectiveEmojiKeywords(const QString& emojiStr,
                                                   const nlohmann::json& jsonKeywords,
                                                   const nlohmann::json& jsonUserEmojis)
{
    const auto it = jsonUserEmojis.find(emojiStr.toStdString());
    auto jsonUserKeywords = nlohmann::json::array();
    auto jsonUserExtraKeywords = nlohmann::json::array();

    if (it != jsonUserEmojis.end()) {
        {
            const auto jsonKeywordsIt = it->find("keywords");

            if (jsonKeywordsIt != it->end()) {
                jsonUserKeywords = *jsonKeywordsIt;
            }
        }

        {
            const auto jsonKeywordsIt = it->find("extra-keywords");

            if (jsonKeywordsIt != it->end()) {
                jsonUserExtraKeywords = *jsonKeywordsIt;
            }
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
    const auto jsonEmojis = loadJson(dir, "emojis.json");
    const auto jsonUserEmojis = loadUserEmojisJson();

    for (auto& emojiKeyJsonValPair : jsonEmojis.items()) {
        auto emoji = call([&emojiKeyJsonValPair, &jsonUserEmojis] {
            const auto emojiStr = QString::fromStdString(emojiKeyJsonValPair.key());
            auto& jsonVal = emojiKeyJsonValPair.value();

            return std::make_unique<const Emoji>(emojiStr,
                                                 QString::fromStdString(jsonVal.at("name")),
                                                 effectiveEmojiKeywords(emojiStr,
                                                                        jsonVal.at("keywords"),
                                                                        jsonUserEmojis),
                                                 jsonVal.at("has-skin-tone-support"),
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
                                                     } else if (str == "14.0") {
                                                         return EmojiVersion::V_14_0;
                                                     } else if (str == "15.0") {
                                                         return EmojiVersion::V_15_0;
                                                     } else {
                                                         assert(str == "15.1");
                                                         return EmojiVersion::V_15_1;
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

    const auto jsonCats = loadJson(dir, "cats.json");

    for (auto& jsonCat : jsonCats) {
        _cats.push_back(call([this, &jsonCat] {
            return std::make_unique<EmojiCat>(QString::fromStdString(jsonCat.at("id")),
                                              QString::fromStdString(jsonCat.at("name")),
                                              call([this, &jsonCat] {
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
    const auto pngLocationsJson = loadJson(dir,
                                           qFmtFormat("emojis-png-locations-{}.json", this->emojiSizeInt()));

    for (auto& keyJsonValPair : pngLocationsJson.items()) {
        auto& jsonLoc = keyJsonValPair.value();

        _emojiPngLocations[_emojis[QString::fromStdString(keyJsonValPair.key())].get()] = {
            static_cast<unsigned int>(jsonLoc.at(0)),
            static_cast<unsigned int>(jsonLoc.at(1))
        };
    }
}

void EmojiDb::findEmojis(QString catName, const QString& needlesStr,
                         std::vector<const Emoji *>& results) const
{
    // split `needlesStr` into individual needles
    const auto needles = needlesStr.trimmed().toLower().split(" +");

    if (needles.isEmpty()) {
        // nothing to search
        return;
    }

    // trim category
    catName = catName.trimmed();

    // clear temporary results
    _tmpFoundEmojis.clear();

    for (auto& cat : _cats) {
        if (!catName.isEmpty() && !cat->name().toLower().contains(catName)) {
            // we don't want to search this category
            continue;
        }

        for (auto emoji : cat->emojis()) {
            auto select = true;

            for (auto& keyword : emoji->keywords()) {
                select = true;

                for (auto& needle : needles) {
                    assert(!needle.isEmpty());

                    if (!keyword.contains(needle)) {
                        // this keyword does not contain this needle
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
    if (!_recentEmojisCat) {
        return;
    }

    _recentEmojisCat->emojis() = std::move(emojis);

    if (_recentEmojisCat->emojis().size() > _maxRecentEmojis) {
        _recentEmojisCat->emojis().resize(_maxRecentEmojis);
    }
}

void EmojiDb::addRecentEmoji(const Emoji& emoji)
{
    if (!_recentEmojisCat) {
        return;
    }

    auto& emojis = _recentEmojisCat->emojis();

    while (true) {
        auto existingIt = std::find(emojis.begin(), emojis.end(), &emoji);

        if (existingIt == emojis.end()) {
            break;
        }

        emojis.erase(existingIt);
    }

    emojis.insert(emojis.begin(), &emoji);

    if (emojis.size() > _maxRecentEmojis) {
        emojis.resize(_maxRecentEmojis);
    }
}

} // namespace jome
