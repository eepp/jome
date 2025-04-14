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
#include <set>
#include <QString>
#include <nlohmann/json.hpp>

namespace jome {

/*
 * Supported Emoji standard versions.
 */
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

/*
 * A single emoji.
 *
 * Contains its string, name, keywords, Emoji version, and whether or
 * not it supports skin tone modifiers.
 *
 * str() and codepoints() provide the UTF-8 string and codepoints with
 * optional skin tone and VS-16 removal.
 */
class Emoji final
{
public:
    // single codepoint
    using Codepoint = unsigned int;

    // sequence of codepoints
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
    /*
     * Builds an emoji having the string `str`, the name `name`, the
     * keywords `keywords`, the Emoji version `version`, and skin tone
     * support if `hasSkinToneSupport` is true.
     *
     * `str` may contain VS-16 codepoints: str() and codepoints()
     * remove them on demand.
     */
    explicit Emoji(QString str, QString name,
                   std::unordered_set<QString>&& keywords, bool hasSkinToneSupport,
                   EmojiVersion version);

    /*
     * Returns the UTF-8 string of this emoji with:
     *
     * • If `skinTone` is set, the skin tone modifier for `*skinTone`.
     * • If `withVs16` is false, without any VS-16 codepoint.
     */
    QString str(const boost::optional<SkinTone>& skinTone = boost::none,
                bool withVs16 = true) const;

    /*
     * Returns the codepoints of this emoji with:
     *
     * • If `skinTone` is set, the skin tone modifier for `*skinTone`.
     *
     *   If `skinTone` is set, then this emoji must have skin tone
     *   support (hasSkinToneSupport() returns true).
     *
     * • If `withVs16` is false, without any VS-16 codepoint.
     */
    Codepoints codepoints(const boost::optional<SkinTone>& skinTone = boost::none,
                          bool withVs16 = true) const;

    /*
     * Name of this emoji.
     */
    const QString& name() const noexcept
    {
        return _name;
    }

    /*
     * Lowercase name of this emoji.
     */
    const QString& lcName() const noexcept
    {
        return _lcName;
    }

    /*
     * Codepoint string (lowercase).
     *
     * For example: `u+2600 u+fe0f`.
     *
     * The purpose of this string is to be able to find an emoji quickly
     * by codepoint with the `U+ABCD` notation.
     */
    const QString& codepointStr() const noexcept
    {
        return _cpStr;
    }

    /*
     * Keywords of this emoji.
     */
    const std::unordered_set<QString>& keywords() const noexcept
    {
        return _keywords;
    }

    /*
     * True if this emoji supports skin tone modifiers.
     */
    bool hasSkinToneSupport() const noexcept
    {
        return _hasSkinToneSupport;
    }

    /*
     * Emoji version of this emoji.
     */
    EmojiVersion version() const noexcept
    {
        return _version;
    }

private:
    const QString _str;
    const QString _name;
    const QString _lcName;
    const QString _cpStr;
    const std::unordered_set<QString> _keywords;
    const bool _hasSkinToneSupport;
    const EmojiVersion _version;
};

/*
 * A category of emojis.
 *
 * A category has an ID, a name, and a list of included emojis.
 *
 * A category doesn't own emojis because more than one category may
 * contain the same emoji. For a given category CAT, the owner of its
 * emojis is the database containing the CAT. This is safe because the
 * owner of CAT is also the same database.
 *
 * The order of `_emojis` is significant: it's the expected
 * presentation order.
 */
class EmojiCat final
{
public:
    /*
     * Builds an empty category having the ID `id` and the name `name`.
     */
    explicit EmojiCat(QString id, QString name);

    /*
     * Builds a category having the ID `id`, the name `name`, and the
     * emojis `emojis`.
     */
    explicit EmojiCat(QString id, QString name, std::vector<const Emoji *>&& emojis);

    /*
     * ID of this category.
     */
    const QString& id() const noexcept
    {
        return _id;
    }

    /*
     * True if this is the "Recent" category.
     */
    bool isRecent() const noexcept
    {
        return _id == "recent";
    }

    /*
     * Name of this category.
     */
    const QString& name() const noexcept
    {
        return _name;
    }

    /*
     * Lowercase name of this category.
     */
    const QString& lcName() const noexcept
    {
        return _lcName;
    }

    /*
     * Emojis of this category.
     */
    std::vector<const Emoji *>& emojis() noexcept
    {
        return _emojis;
    }

    /*
     * Emojis of this category.
     */
    const std::vector<const Emoji *>& emojis() const noexcept
    {
        return _emojis;
    }

private:
    const QString _id;
    const QString _name;
    const QString _lcName;
    std::vector<const Emoji *> _emojis;
};

/*
 * The location of the top-left corner of an emoji within a PNG image.
 */
struct EmojisPngLocation final
{
    unsigned int x;
    unsigned int y;
};

/*
 * An emoji database.
 *
 * An emoji database contains all the emojis (once) as `Emoji` instances
 * as well as a list of categories (`EmojiCat` instances), each category
 * containing a list of pointers to those same emojis. Get all the
 * categories with cats() and all the emojis with emojis(). Get the
 * `Emoji` instance of its corresponding string with emojiForStr().
 * Check whether or not the database contains an emoji having some
 * string with hasEmoji().
 *
 * An emoji database also provides emoji images as locations within a
 * PNG images containing all the emoji images. Use emojiSizeInt(),
 * emojisPngPath(), and emojiPngLocations().
 *
 * Find emojis by category and terms with findEmojis().
 *
 * Add a recent emoji to the "Recent" category with addRecentEmoji().
 * Get all the recent emojis with recentEmojis(). Get the "Recent"
 * category with recentEmojisCat().
 */
class EmojiDb final
{
public:
    /*
     * Supported emoji image sizes.
     */
    enum class EmojiSize
    {
        Size16 = 16,
        Size24 = 24,
        Size32 = 32,
        Size40 = 40,
        Size48 = 48,
    };

public:
    /*
     * Builds an emoji database using the data (asset) directory `dir`
     * and the emoji image size `emojiSize`.
     *
     * At most `maxRecentEmojis` are retrieved from settings if
     * `noRecentCat` is false.
     */
    explicit EmojiDb(const QString& dir, EmojiSize emojiSize,
                     unsigned int maxRecentEmojis, bool noRecentCat,
                     bool incRecentInFindResults);

    /*
     * Appends the emojis found with the partial category name `cat` and
     * the find terms `needles` to `results`.
     */
    void findEmojis(QString cat, const QString& needles,
                    std::vector<const Emoji *>& results) const;

    /*
     * All the recent emojis.
     */
    void recentEmojis(std::vector<const Emoji *>&& emojis);

    /*
     * Adds the emoji `emoji` as the most recent emoji of the
     * "Recent" category.
     *
     * This method only affects the database itself: it doesn't
     * update settings.
     */
    void addRecentEmoji(const Emoji& emoji);

    /*
     * Configured emoji image size.
     */
    EmojiSize emojiSize() const
    {
        return _emojiSize;
    }

    /*
     * Integral configured emoji image size.
     */
    unsigned int emojiSizeInt() const
    {
        return static_cast<unsigned int>(_emojiSize);
    }

    /*
     * Path to the PNG image containing all the emojis of
     * size emojiSize().
     */
    const QString& emojisPngPath() const noexcept
    {
        return _emojisPngPath;
    }

    /*
     * All the categories.
     */
    const std::vector<std::unique_ptr<EmojiCat>>& cats() const noexcept
    {
        return _cats;
    }

    /*
     * Map of emoji strings to emojis.
     *
     * The emoji strings (keys) may contain VS-16 codepoints.
     */
    const std::unordered_map<QString, std::unique_ptr<const Emoji>>& emojis() const noexcept
    {
        return _emojis;
    }

    /*
     * "Recent" category, or `nullptr` if none.
     */
    const EmojiCat *recentEmojisCat() const noexcept
    {
        return _recentEmojisCat;
    }

    /*
     * Returns the emoji for the string `str`.
     *
     * `str` may contain VS-16 codepoints.
     */
    const Emoji& emojiForStr(const QString& str) const
    {
        return *_emojis.at(str);
    }

    /*
     * Returns whether or not an emoji has the exact string `str`.
     *
     * `str` may contain VS-16 codepoints.
     */
    bool hasEmoji(const QString& str) const
    {
        return _emojis.count(str) >= 1;
    }

    /*
     * Map of emojis to corresponding PNG locations within the
     * file emojisPngPath().
     *
     * The locations locate the top-left pixel of the emoji and its
     * width and height within the whole image is emojiSizeInt().
     */
    const std::unordered_map<const Emoji *, EmojisPngLocation>& emojiPngLocations() const noexcept
    {
        return _emojiPngLocations;
    }

private:
    /*
     * Temporary find results.
     */
    struct _FindResult final
    {
        // score of the result
        unsigned int score;

        // original (global), unique position of the emoji
        unsigned int pos;

        // found emoji
        const Emoji *emoji;

        bool operator<(const _FindResult& other) const noexcept
        {
            if (score == other.score) {
                // same score: fall back to global position
                return pos < other.pos;
            }

            return score < other.score;
        }
    };

private:
    /*
     * Fills `_emojis` from the assets found in `dir`.
     */
    void _createEmojis(const QString& dir);

    /*
     * Fills `_cats` from the assets found in `dir`.
     *
     * Doesn't add a "Recent" category if `noRecentCat` is true.
     */
    void _createCats(const QString& dir, bool noRecentCat);

    /*
     * Fills `_emojiPngLocations` from the assets found in `dir`.
     */
    void _createEmojiPngLocations(const QString& dir);

private:
    const EmojiSize _emojiSize;
    const QString _emojisPngPath;
    std::vector<std::unique_ptr<EmojiCat>> _cats;
    std::unordered_map<QString, std::unique_ptr<const Emoji>> _emojis;
    std::unordered_map<const Emoji *, EmojisPngLocation> _emojiPngLocations;
    mutable std::set<_FindResult> _tmpFindResults;
    mutable std::set<const Emoji *> _tmpFindResultEmojis;
    EmojiCat *_recentEmojisCat = nullptr;
    unsigned int _maxRecentEmojis;
    bool _incRecentInFindResults;
};

} // namespace jome

#endif // _JOME_EMOJI_DB_HPP
