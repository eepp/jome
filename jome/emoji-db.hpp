/*
 * Copyright (C) 2019-2025 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_EMOJI_DB_HPP
#define _JOME_EMOJI_DB_HPP

#include <boost/optional/optional.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <QString>
#include <nlohmann/json.hpp>

namespace jome {

enum class EmojiVersion
{
    V_0_6,
    V_0_7,
    V_1_0,
    V_2_0,
    V_3_0,
    V_4_0,
    V_5_0,
    V_11_0,
    V_12_0,
    V_12_1,
    V_13_0,
    V_13_1,
    V_14_0,
    V_15_0,
    V_15_1,
};

class Emoji final
{
public:
    using Codepoint = unsigned int;
    using Codepoints = std::vector<Codepoint>;

public:
    enum class SkinTone
    {
        Light,
        MediumLight,
        Medium,
        MediumDark,
        Dark,
    };

public:
    explicit Emoji(QString str, QString name,
                   std::unordered_set<QString>&& keywords, bool hasSkinToneSupport,
                   EmojiVersion version);

    QString str(const boost::optional<SkinTone>& skinTone = boost::none,
                bool withVs16 = true) const;

    Codepoints codepoints(const boost::optional<SkinTone>& skinTone = boost::none,
                          bool withVs16 = true) const;

    const QString& name() const noexcept
    {
        return _name;
    }

    const std::unordered_set<QString>& keywords() const noexcept
    {
        return _keywords;
    }

    bool hasSkinToneSupport() const noexcept
    {
        return _hasSkinToneSupport;
    }

    EmojiVersion version() const noexcept
    {
        return _version;
    }

private:
    const QString _str;
    const QString _name;
    const std::unordered_set<QString> _keywords;
    const bool _hasSkinToneSupport;
    const EmojiVersion _version;
};

class EmojiCat final
{
public:
    explicit EmojiCat(QString id, QString name);

    explicit EmojiCat(QString id, QString name, std::vector<const Emoji *>&& emojis);

    const QString& id() const noexcept
    {
        return _id;
    }

    const QString& name() const noexcept
    {
        return _name;
    }

    std::vector<const Emoji *>& emojis() noexcept
    {
        return _emojis;
    }

    const std::vector<const Emoji *>& emojis() const noexcept
    {
        return _emojis;
    }

private:
    const QString _id;
    const QString _name;
    std::vector<const Emoji *> _emojis;
};

struct EmojisPngLocation final
{
    unsigned int x;
    unsigned int y;
};

class EmojiDb final
{
public:
    enum class EmojiSize
    {
        Size16 = 16,
        Size24 = 24,
        Size32 = 32,
        Size40 = 40,
        Size48 = 48,
    };

public:
    explicit EmojiDb(const QString& dir, EmojiSize emojiSize,
                     unsigned int maxRecentEmojis, bool noRecentCat);

    void findEmojis(QString cat, const QString& needles,
                    std::vector<const Emoji *>& results) const;

    void recentEmojis(std::vector<const Emoji *>&& emojis);
    void addRecentEmoji(const Emoji& emoji);

    EmojiSize emojiSize() const
    {
        return _emojiSize;
    }

    unsigned int emojiSizeInt() const
    {
        return static_cast<unsigned int>(_emojiSize);
    }

    const QString& emojisPngPath() const noexcept
    {
        return _emojisPngPath;
    }

    const std::vector<std::unique_ptr<EmojiCat>>& cats() const noexcept
    {
        return _cats;
    }

    const std::unordered_map<QString, std::unique_ptr<const Emoji>>& emojis() const noexcept
    {
        return _emojis;
    }

    const EmojiCat *recentEmojisCat() const noexcept
    {
        return _recentEmojisCat;
    }

    const Emoji& emojiForStr(const QString& str) const
    {
        return *_emojis.at(str);
    }

    bool hasEmoji(const QString& str) const
    {
        return _emojis.count(str) >= 1;
    }

    const std::unordered_set<QString>& keywords() const noexcept
    {
        return _keywords;
    }

    const std::unordered_map<const Emoji *, EmojisPngLocation>& emojiPngLocations() const noexcept
    {
        return _emojiPngLocations;
    }

private:
    void _createEmojis(const QString& dir);
    void _createCats(const QString& dir, bool noRecentCat);
    void _createEmojiPngLocations(const QString& dir);

private:
    const EmojiSize _emojiSize;
    const QString _emojisPngPath;
    std::vector<std::unique_ptr<EmojiCat>> _cats;
    std::unordered_map<QString, std::unique_ptr<const Emoji>> _emojis;
    std::unordered_set<QString> _keywords;
    std::unordered_map<const Emoji *, EmojisPngLocation> _emojiPngLocations;
    mutable std::unordered_set<const Emoji *> _tmpFoundEmojis;
    EmojiCat *_recentEmojisCat = nullptr;
    unsigned int _maxRecentEmojis;
};

} // namespace jome

#endif // _JOME_EMOJI_DB_HPP
