--
-- Just a room with a bunch of barrels to shoot
--

local level_add_wall = function(lvl, x1, y1, x2, y2, texid)
	lvl.walls[#lvl.walls+1] = {x1 = x1, y1 = y1, x2 = x2, y2 = y2, texid = texid}
end

local level_add_sprite = function(lvl, x, y, texid)
	lvl.sprites[#lvl.sprites+1] = {x = x, y = y, texid = texid}
end

local level = {
	walls = {},
	sprites = {},
	-- player starts in middle of 5x5 box
	player_start = {x = 2.5, y = 2.5}
}

-- Outer walls (5x5 box room)
level_add_wall(level, 0, 0, 5, 0, 1)
level_add_wall(level, 5, 0, 5, 5, 2)
level_add_wall(level, 5, 5, 0, 5, 1)
level_add_wall(level, 0, 5, 0, 0, 2)

-- Layer of barrels close to wall in middle of tiles
level_add_sprite(level, 0.5, 0.5, 8)
level_add_sprite(level, 1.5, 0.5, 8)
level_add_sprite(level, 2.5, 0.5, 8)
level_add_sprite(level, 3.5, 0.5, 8)
level_add_sprite(level, 4.5, 0.5, 8)

-- Layer of barrels in front on the diagonals
level_add_sprite(level, 1, 1, 8)
level_add_sprite(level, 2, 1, 8)
level_add_sprite(level, 3, 1, 8)
level_add_sprite(level, 4, 1, 8)

return level
