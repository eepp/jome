/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_EMOJI_DB_HPP
#define _JOME_EMOJI_DB_HPP

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "simple-json.hpp"

namespace jome {

class Emoji
{
public:
    using Codepoint = char32_t;
    using Codepoints = std::vector<Codepoint>;

public:
    enum class SkinTone {
        NONE,
        LIGHT,
        MEDIUM_LIGHT,
        MEDIUM,
        MEDIUM_DARK,
        DARK,
    };

public:
    explicit Emoji(const std::string& str, const std::string& name,
                   std::unordered_set<std::string>&& keywords,
                   bool hasSkinToneSupport);
    Codepoints codepoints() const;
    Codepoints codepointsWithSkinTone(SkinTone skinTone) const;
    std::string strWithSkinTone(SkinTone skinTone) const;
    const std::string& lcName() const;

    const std::string& str() const noexcept
    {
        return _str;
    }


    const std::string& name() const noexcept
    {
        return _name;
    }

    const std::unordered_set<std::string>& keywords() const noexcept
    {
        return _keywords;
    }

    bool hasSkinToneSupport() const noexcept
    {
        return _hasSkinToneSupport;
    }

private:
    const std::string _str;
    const std::string _name;
    mutable std::string _lcName;
    const std::unordered_set<std::string> _keywords;
    const bool _hasSkinToneSupport;
};

class EmojiCat
{
public:
    explicit EmojiCat(const std::string& id, const std::string& name,
                      std::vector<const Emoji *>&& emojis);
    const std::string& lcName() const;

    const std::string& id() const noexcept
    {
        return _id;
    }

    const std::string& name() const noexcept
    {
        return _name;
    }

    const std::vector<const Emoji *>& emojis() const noexcept
    {
        return _emojis;
    }

private:
    const std::string _id;
    const std::string _name;
    mutable std::string _lcName;
    const std::vector<const Emoji *> _emojis;
};

struct EmojisPngLocation
{
    unsigned int x;
    unsigned int y;
};

class EmojiDb
{
public:
    explicit EmojiDb(const std::string& dir);
    void findEmojis(const std::string& cat, const std::string& needles,
                    std::vector<const Emoji *>& results) const;

    const std::string& emojisPngPath() const noexcept
    {
        return _emojisPngPath;
    }

    const std::vector<std::unique_ptr<const EmojiCat>>& cats() const noexcept
    {
        return _cats;
    }

    const std::unordered_map<std::string, std::unique_ptr<const Emoji>>& emojis() const noexcept
    {
        return _emojis;
    }

    const Emoji& emojiForStr(const std::string& str) const
    {
        return *_emojis.at(str);
    }

    const std::unordered_set<std::string>& keywords() const noexcept
    {
        return _keywords;
    }

    const std::unordered_map<const Emoji *, EmojisPngLocation>& emojiPngLocations() const noexcept
    {
        return _emojiPngLocations;
    }

    const std::unordered_set<const Emoji *>& emojisForKeyword(const std::string& keyword) const
    {
        return _keywordEmojis.find(keyword)->second;
    }

private:
    json::JSON _loadJson(const std::string& dir, const std::string& file);
    void _createEmojis(const std::string& dir);
    void _createCats(const std::string& dir);
    void _createEmojiPngLocations(const std::string& dir);

private:
    const std::string _emojisPngPath;
    std::vector<std::unique_ptr<const EmojiCat>> _cats;
    std::unordered_map<std::string, std::unique_ptr<const Emoji>> _emojis;
    std::unordered_map<std::string, std::unordered_set<const Emoji *>> _keywordEmojis;
    std::unordered_set<std::string> _keywords;
    std::unordered_map<const Emoji *, EmojisPngLocation> _emojiPngLocations;
    mutable std::vector<std::string> _tmpNeedles;
    mutable std::unordered_set<const Emoji *> _tmpFoundEmojis;
};

} // namespace jome

#endif // _JOME_EMOJI_DB_HPP
