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
    def __init__(self, emoji, name, keywords, has_skin_tone_support, version):
        self._emoji = emoji
        self._name = name
        self._keywords = keywords
        self._has_skin_tone_support = has_skin_tone_support
        self._version = version

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

    @property
    def version(self):
        return self._version


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


def _get_emoji_descriptors(version):
    emojis_with_skin_tone_support = set(_extract_emojis_from_file('with-skin-tone-support.txt'))

    with open(f'emojis-{version}.txt') as f:
        emojis_txt = f.read()

    blocks = re.split(r'\n{2,}', emojis_txt)
    emoji_descrs = {}

    for block in blocks:
        block = block.strip()

        if len(block) == 0:
            continue

        lines = block.split('\n')
        assert len(lines) == 3
        emoji = lines[0].strip()
        name = lines[1].strip()
        kws = [kw.strip() for kw in lines[2].split('|')]
        emoji_descrs[emoji] = _EmojiDescriptor(emoji, name, kws,
                                               emoji in emojis_with_skin_tone_support,
                                               version)

    return emoji_descrs


def _get_all_emoji_descriptors():
    emoji_descrs = {}
    versions = [
        '0.6',
        '0.7',
        '1.0',
        '2.0',
        '3.0',
        '4.0',
        '5.0',
        '11.0',
        '12.0',
        '12.1',
        '13.0',
        '13.1',
        '14.0',
        '15.0',
    ]

    for version in versions:
        emoji_descrs.update(_get_emoji_descriptors(version))

    return emoji_descrs


def _get_cats_yml():
    with open('cats.yml') as f:
        return yaml.load(f, Loader=yaml.Loader)


def _gen_emojis_json(output_dir, emoji_descriptors):
    emojis_json = {}

    for emoji_descriptor in emoji_descriptors:
        emojis_json[emoji_descriptor.emoji] = {
            'name': emoji_descriptor.name,
            'keywords': emoji_descriptor.keywords,
            'has-skin-tone-support': emoji_descriptor.has_skin_tone_support,
            'version': emoji_descriptor.version,
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


def _gen_emoji_pngs_from_svgs_one_size(output_dir, size):
    png_dir = os.path.join(output_dir, 'twemoji-png-{}'.format(size))

    if os.path.exists(png_dir):
        return

    os.makedirs(png_dir, exist_ok=True)

    for file_name in os.listdir('twemoji-svg'):
        if not file_name.endswith('.svg'):
            continue

        file_name_no_ext = file_name[:-4]
        cairosvg.svg2png(url=os.path.join('twemoji-svg', file_name),
                         write_to=os.path.join(png_dir, '{}.png'.format(file_name_no_ext)),
                         parent_width=size, parent_height=size)


def _gen_emoji_pngs_from_svgs(output_dir, sizes):
    for size in sizes:
        _gen_emoji_pngs_from_svgs_one_size(output_dir, size)


def _get_emoji_png_file_names(emoji):
    codepoints = ['{:x}'.format(ord(c)) for c in emoji]
    file_names = ['-'.join(codepoints) + '.png']

    if 'fe0f' in codepoints:
        codepoints = [c for c in codepoints if c != 'fe0f']
        file_names.append('-'.join(codepoints) + '.png')

    return file_names


def _gen_emojis_png_one_size(output_dir, emoji_descriptors, size):
    cols = 32
    rows = len(emoji_descriptors) // cols + 1
    locations = {}
    col = 0
    row = 0
    out_surf = cairo.ImageSurface(cairo.Format.ARGB32, cols * size,
                                  rows * size)
    cr = cairo.Context(out_surf)

    for emoji_descr in emoji_descriptors:
        emoji = emoji_descr.emoji
        file_names = _get_emoji_png_file_names(emoji)
        existing_path = None

        for file_name in file_names:
            path = os.path.join(output_dir, 'twemoji-png-{}'.format(size),
                                file_name)

            if os.path.exists(path):
                existing_path = path

        if existing_path is None:
            _error('Cannot find PNG file for emoji `{}`; candidates are {}.'.format(emoji, file_names))

        emoji_surf = cairo.ImageSurface.create_from_png(existing_path)
        loc_x = col * size
        loc_y = row * size
        cr.save()
        cr.translate(loc_x, loc_y)
        cr.set_source_surface(emoji_surf)
        cr.rectangle(0, 0, size, size)
        cr.fill()
        cr.restore()
        emoji_surf.finish()
        locations[emoji] = [loc_x, loc_y]

        if col == cols - 1:
            col = 0
            row += 1
        else:
            col += 1

    out_surf.write_to_png(os.path.join(output_dir, 'emojis-{}.png'.format(size)))
    out_surf.finish()

    with open(os.path.join(output_dir, 'emojis-png-locations-{}.json'.format(size)), 'w') as f:
        json.dump(locations, f, ensure_ascii=False, indent=2)


def _gen_emojis_png(output_dir, emoji_descriptors, sizes):
    for size in sizes:
        _gen_emojis_png_one_size(output_dir, emoji_descriptors, size)


def _main(output_dir):
    os.makedirs(output_dir, exist_ok=True)
    all_emoji_descrs = _get_all_emoji_descriptors()
    cats_yml = _get_cats_yml()
    emoji_descriptors = []
    categories = []
    done_emojis = set()

    for cat_yml in cats_yml:
        cat_id = cat_yml['id']
        cat_name = cat_yml['name']
        cat = _Category(cat_id, cat_name)
        emojis = _extract_cat_emojis(cat_id)

        for emoji in emojis:
            if emoji not in done_emojis:
                if emoji not in all_emoji_descrs:
                    _error('Cannot find `{}` in the `emojis-*.txt` files.'.format(emoji))

                emoji_descriptors.append(all_emoji_descrs[emoji])
                done_emojis.add(emoji)

            if emoji not in cat.emojis:
                cat.emojis.append(emoji)

        categories.append(cat)

    print('Creating `emojis.json`')
    _gen_emojis_json(output_dir, emoji_descriptors)
    print('Creating `cats.json`')
    _gen_cats_json(output_dir, categories)
    sizes = [16, 24, 32, 40, 48]
    print('Creating `twemoji-png-*` directories')
    _gen_emoji_pngs_from_svgs(output_dir, sizes)
    print('Creating `emojis-*.png` and `emojis-png-locations-*.json`')
    _gen_emojis_png(output_dir, emoji_descriptors, sizes)


if __name__ == '__main__':
    if len(sys.argv) < 2:
        _error('Specify output directory.')

    _main(sys.argv[1])
