# Copyright (C) 2019 Philippe Proulx <eepp.ca>
#
# This software may be modified and distributed under the terms
# of the MIT license. See the LICENSE file for details.

import os
import re
import json
import yaml
import sys
import cairosvg
import cairo
import os.path


class _EmojiDescriptor:
    def __init__(self, emoji, name, keywords, has_skin_tone_support):
        self._emoji = emoji
        self._name = name
        self._keywords = keywords
        self._has_skin_tone_support = has_skin_tone_support

    @property
    def emoji(self):
        return self._emoji

    @property
    def name(self):
        return self._name

    @property
    def keywords(self):
        return self._keywords

    @property
    def has_skin_tone_support(self):
        return self._has_skin_tone_support


class _Category:
    def __init__(self, id, name):
        self._id = id
        self._name = name
        self._emojis = []

    @property
    def id(self):
        return self._id

    @property
    def name(self):
        return self._name

    @property
    def emojis(self):
        return self._emojis


class _EmojiJsonEntry:
    def __init__(self, emoji, name, keywords):
        self._emoji = emoji
        self._name = name
        self._keywords = keywords

    @property
    def emoji(self):
        return self._emoji

    @property
    def name(self):
        return self._name

    @property
    def keywords(self):
        return self._keywords


def _error(msg):
    print('Error: {}'.format(msg), file=sys.stderr)
    sys.exit(1)


def _extract_emojis_from_file(path):
    emojis = []

    with open(path) as f:
        lines = f.read().split('\n')

        for line in lines:
            if not line:
                continue

            parts = line.split(' ')

            if not parts:
                continue

            emojis.append(parts[0].strip())

    return emojis


def _extract_cat_emojis(cat_id):
    return _extract_emojis_from_file('cats/{}.txt'.format(cat_id))


def _get_emoji_json_entries():
    with open('emoji.json') as f:
        emoji_json = json.load(f)

    entries = {}

    for entry in emoji_json:
        emoji = entry['char']
        name = entry['name'][0].upper() + entry['name'][1:]
        keywords = set([name.lower()])
        prev_codepoint = None

        for keyword in entry['keywords'].split('|'):
            keywords.add(keyword.strip().lower())

        entries[emoji] = _EmojiJsonEntry(emoji, name, list(keywords))

    return entries


def _get_cats_yml():
    with open('cats.yml') as f:
        return yaml.load(f)


def _gen_emojis_json(output_dir, emoji_descriptors):
    emojis_json = {}

    for emoji_descriptor in emoji_descriptors:
        emojis_json[emoji_descriptor.emoji] = {
            'name': emoji_descriptor.name,
            'keywords': emoji_descriptor.keywords,
            'has-skin-tone-support': emoji_descriptor.has_skin_tone_support,
        }

    with open(os.path.join(output_dir, 'emojis.json'), 'w') as f:
        json.dump(emojis_json, f, ensure_ascii=False, indent=2)


def _gen_cats_json(output_dir, categories):
    cats_json = []

    for cat in categories:
        cats_json.append({
            'id': cat.id,
            'name': cat.name,
            'emojis': cat.emojis,
        })

    with open(os.path.join(output_dir, 'cats.json'), 'w') as f:
        json.dump(cats_json, f, ensure_ascii=False, indent=2)


def _gen_emoji_pngs_from_svgs(output_dir):
    png_dir = os.path.join(output_dir, 'twemoji-png-32')

    if os.path.exists(png_dir):
        return

    os.makedirs(png_dir, exist_ok=True)

    for file_name in os.listdir('twemoji-svg'):
        if not file_name.endswith('.svg'):
            continue

        file_name_no_ext = file_name[:-4]
        cairosvg.svg2png(url=os.path.join('twemoji-svg', file_name),
                         write_to=os.path.join(png_dir, '{}.png'.format(file_name_no_ext)),
                         parent_width=32, parent_height=32)


def _get_emoji_png_file_names(emoji):
    codepoints = ['{:x}'.format(ord(c)) for c in emoji]
    file_names = ['-'.join(codepoints) + '.png']

    if 'fe0f' in codepoints:
        codepoints = [c for c in codepoints if c != 'fe0f']
        file_names.append('-'.join(codepoints) + '.png')

    return file_names


def _gen_emojis_png(output_dir, emoji_descriptors):
    cols = 32
    rows = len(emoji_descriptors) // cols + 1
    locations = {}
    col = 0
    row = 0
    out_surf = cairo.ImageSurface(cairo.Format.ARGB32, cols * 32, rows * 32)
    cr = cairo.Context(out_surf)

    for emoji_descr in emoji_descriptors:
        emoji = emoji_descr.emoji
        file_names = _get_emoji_png_file_names(emoji)
        existing_path = None

        for file_name in file_names:
            path = os.path.join(output_dir, 'twemoji-png-32', file_name)

            if os.path.exists(path):
                existing_path = path

        if existing_path is None:
            _error('Cannot find PNG file for emoji `{}`; candidates are {}.'.format(emoji, file_names))

        emoji_surf = cairo.ImageSurface.create_from_png(existing_path)
        loc_x = col * 32
        loc_y = row * 32
        cr.save()
        cr.translate(loc_x, loc_y)
        cr.set_source_surface(emoji_surf)
        cr.rectangle(0, 0, 32, 32)
        cr.fill()
        cr.restore()
        emoji_surf.finish()
        locations[emoji] = [loc_x, loc_y]

        if col == cols - 1:
            col = 0
            row += 1
        else:
            col += 1

    out_surf.write_to_png(os.path.join(output_dir, 'emojis.png'))
    out_surf.finish()

    with open(os.path.join(output_dir, 'emojis-png-locations.json'), 'w') as f:
        json.dump(locations, f, ensure_ascii=False, indent=2)


def _main(output_dir):
    os.makedirs(output_dir, exist_ok=True)
    emoji_json_entries = _get_emoji_json_entries()
    cats_yml = _get_cats_yml()
    emoji_descriptors = []
    categories = []
    done_emojis = set()
    emojis_with_skin_tone_support = set(_extract_emojis_from_file('with-skin-tone-support.txt'))

    for cat_yml in cats_yml:
        cat_id = cat_yml['id']
        cat_name = cat_yml['name']
        cat = _Category(cat_id, cat_name)
        emojis = _extract_cat_emojis(cat_id)

        for emoji in emojis:
            if emoji not in done_emojis:
                if emoji not in emoji_json_entries:
                    _error('Cannot find `{}` in `emoji.json`.'.format(emoji))

                emoji_json_entry = emoji_json_entries[emoji]
                has_skin_tone_support = emoji in emojis_with_skin_tone_support
                emoji_descr = _EmojiDescriptor(emoji, emoji_json_entry.name,
                                               emoji_json_entry.keywords,
                                               has_skin_tone_support)
                emoji_descriptors.append(emoji_descr)
                done_emojis.add(emoji)

            if emoji not in cat.emojis:
                cat.emojis.append(emoji)

        categories.append(cat)

    print('Creating `emojis.json`')
    _gen_emojis_json(output_dir, emoji_descriptors)
    print('Creating `cats.json`')
    _gen_cats_json(output_dir, categories)
    print('Creating `twemoji-png-32`')
    _gen_emoji_pngs_from_svgs(output_dir)
    print('Creating `emojis.png` and `emojis-png-locations.json`')
    _gen_emojis_png(output_dir, emoji_descriptors)


if __name__ == '__main__':
    if len(sys.argv) < 2:
        _error('Specify output directory.')

    _main(sys.argv[1])
