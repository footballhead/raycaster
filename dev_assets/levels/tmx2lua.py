#!/usr/bin/python

import xml.etree.ElementTree
import sys


def parse_polyline(polyline):
    """Return a list of {x, y} objects for the polyline"""
    points = []

    points_attribute = polyline.get('points')
    tuples = points_attribute.split(' ')
    for tup in tuples:
        components = tup.split(',')
        points.append({'x': float(components[0]), 'y': float(components[1])})

    return points


def parse_properties(properties):
    """ Return a map of all object properties, None if no properties """

    all_props = {}

    for prop in properties:
        key = prop.get('name')

        if not key or key == 'Comment':
            print('Ignoring key: `{}`'.format(key))
            continue

        val = prop.get('value')
        if not val:
            print('Ignorning key with malformed value: `{}`'.format(key))

        all_props[key] = val

    return all_props


def parse_wall(obj):
    """Go through object and pick out all the wall-specific stuff"""

    props = []
    points = []
    walls = []

    origin_x = float(obj.get('x'))
    origin_y = float(obj.get('y'))

    for child in obj:
        if child.tag == 'properties':
            props = parse_properties(child)
        elif child.tag == 'polyline':
            points = parse_polyline(child)

    # Adjust all the points to be in world space
    for point in points:
        point['x'] += origin_x
        point['y'] += origin_y

    for i in range(1, len(points)):
        walls.append({
            'x1': points[i-1]['x'],
            'y1': points[i-1]['y'],
            'x2': points[i]['x'],
            'y2': points[i]['y'],
            'texid': int(props['texid']),
            })

    return walls


def parse_sprite(sprite):
    """Go through object and pick out all sprite stuff"""

    props = []

    for child in sprite:
        if child.tag == 'properties':
            props = parse_properties(child)

    return {'x': float(sprite.get('x')),
        'y': float(sprite.get('y')),
        'texid': int(props['texid'])}

def parse_objectgroup(objectgroup):
    """Go through all objects and translate into desired format"""
    walls = []
    sprites = []
    player_start = None

    for obj in objectgroup:
        type_ = obj.get('type')

        if type_ == 'wall':
            walls.extend(parse_wall(obj))
        elif type_ == 'player_start':
            player_start = {'x': float(obj.get('x')), 'y': float(obj.get('y'))}
        elif type_ == 'sprite':
            sprites.append(parse_sprite(obj))

    return player_start, walls, sprites


def main(argv):
    ''' Parse the given file and convert to binary '''

    # Make sure enough args
    program = argv[0]

    if len(argv) < 2:
        sys.stderr.write('Usage: {} file.tmx\n'.format(program))
        return 1

    # Parse the TMX and get the root <map> element
    tmx_file = argv[1]

    sys.stderr.write('Parsing {}...\n'.format(tmx_file)) # DEBUG
    doc = xml.etree.ElementTree.parse(tmx_file)
    root = doc.getroot()

    tilewidth = float(root.get('tilewidth'))
    tileheight = float(root.get('tileheight'))

    player_start = None # 2-tuple: <x, y>
    walls = [] # List of 5-tuples: <x1, y1, x2, y2, texid>
    sprites = [] # List of 3-tuples: <x, y, texid> 

    # Find important layers
    sys.stderr.write('Finding layers...\n') # DEBUG
    for child in root:
        if child.tag == 'objectgroup':
            sys.stderr.write('Parsing objects...\n') # DEBUG
            player_start, walls, sprites = parse_objectgroup(child)

            # Only parse the first objectgroup... for now...
            continue

    # Fix for grid size
    sys.stderr.write('Fixing object coordinates...\n') # DEBUG
    for wall in walls:
        wall['x1'] = wall['x1'] / tilewidth
        wall['y1'] = wall['y1'] / tileheight
        wall['x2'] = wall['x2'] / tilewidth
        wall['y2'] = wall['y2'] / tileheight

    for sprite in sprites:
        sprite['x'] /= tilewidth
        sprite['y'] /= tilewidth

    player_start['x'] = player_start['x'] / tilewidth
    player_start['y'] = player_start['y'] / tileheight

    # Write to disk!
    lua_file = '{}.lua'.format(tmx_file)
    sys.stderr.write('Writing to {}...\n'.format(lua_file))
    with open(lua_file, 'w') as out:
        out.write('return {\n')
        out.write('  player_start = {{x = {}, y = {}}},\n'
            .format(player_start['x'], player_start['y']))
        out.write('  walls = {\n')
        for wall in walls:
            out.write(
                '    {{x1 = {}, y1 = {}, x2 = {}, y2 = {}, texid = {}}},\n'
                .format(wall['x1'], wall['y1'], wall['x2'], wall['y2'],
                    wall['texid']))
        out.write('  },\n')
        out.write('  sprites = {\n')
        for sprite in sprites:
            out.write('    {{x = {}, y = {}, texid = {}}},\n'
                .format(sprite['x'], sprite['y'], sprite['texid']))
        out.write('  },\n')
        out.write('}\n')

    sys.stderr.write('Done!\n')

    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv))

