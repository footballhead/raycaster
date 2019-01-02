local level_add_wall = function(lvl, x1, y1, x2, y2, texid)
	lvl.walls[#lvl.walls+1] = {x1 = x1, y1 = y1, x2 = x2, y2 = y2, texid = texid}
end

local level_add_sprite = function(lvl, x, y, texid)
	lvl.sprites[#lvl.sprites+1] = {x = x, y = y, texid = texid}
end

local level = {
	walls = {},
	sprites = {},
	player_start = {x = 2.5, y = 3.5}
}

-- Outer walls
level_add_wall(level, 1, 1, 2, 1, 1)
level_add_wall(level, 2, 1, 2, 2, 2)
level_add_wall(level, 2, 2, 3, 2, 2)
level_add_wall(level, 3, 2, 3, 1, 2)
level_add_wall(level, 3, 1, 7, 1, 1)
level_add_wall(level, 7, 1, 7, 5, 1)
level_add_wall(level, 7, 5, 4, 5, 2)
level_add_wall(level, 4, 5, 4, 6, 2)
level_add_wall(level, 4, 6, 7, 6, 2)
level_add_wall(level, 7, 6, 7, 7, 1)
level_add_wall(level, 7, 7, 2, 7, 1)
level_add_wall(level, 2, 7, 2, 6, 2)
level_add_wall(level, 2, 6, 1, 6, 2)
level_add_wall(level, 1, 6, 1, 1, 1)
-- Inner island
level_add_wall(level, 5, 2, 6, 2, 2)
level_add_wall(level, 6, 2, 6, 4, 2)
level_add_wall(level, 6, 4, 5, 4, 2)
level_add_wall(level, 5, 4, 5, 2, 2)

-- Column
level_add_sprite(level, 1.5, 1.5, 4)
-- Barrel
level_add_sprite(level, 3.5, 3.5, 8)
-- Test enemy
level_add_sprite(level, 3.5, 2.5, 10)

return level
