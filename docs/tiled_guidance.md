# Tips for making levels in Tiled

## Set grid to 50x50

The tilewidth/tileheight doesn't really matter since it's normalized to floats
by the conversion script, so choose something that shows objects well enough.

## Use only 1 layer: an objectgroup

All objects need to occupy one objectgroup layer.

## Add color to objects with the Object Type Editor

Click _View > Object Types Editor_. Then define types for:

 * wall
 * sprite
 * player\_start

Then choose colors for each type. This will make levels less gray and easier to
read.

## Remove objectgroup offset

This will mess with grid snapping.
