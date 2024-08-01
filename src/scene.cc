Platform PlacePlatform(f32 x, f32 y, f32 width, f32 height){
	Platform plat;
	plat.rec.x = x;
	plat.rec.y = y;
	plat.rec.width = width;
	plat.rec.height = height;
	return plat;
};
Block PlaceBlock(f32 x, f32 y){
	Block blk;
	blk.pos = {x, y};
	return blk;
};
Monster PlaceMonster(f32 x, f32 y, f32 velx=MONSTER_VEL_X, f32 vely=0){
	Monster monster;
	monster.pos = {x, y};
	monster.vel = {velx, vely};
	return monster;
};
void buildScene2(){
	Scene &scene = state->curScene;

	scene.player.pos = {-PLAYER_WIDTH-150, -PLAYER_HEIGHT};
	scene.player.vel = {0.0};
	scene.player.aura = {300, 100};

	scene.door.pos = {-100, -DOOR_HEIGHT};
	scene.key.pos = {state->worldBound.x - KEY_WIDTH - 50, -KEY_HEIGHT};

	scene.plats.push(PlacePlatform(state->worldBound.x-500, 0, 500, 100));
	scene.plats.push(PlacePlatform(-450, 0, 600, 100));
	scene.plats.push(PlacePlatform(-400, -250, 400, 50));

	scene.blocks.push(PlaceBlock(state->worldBound.x-500, -BLOCK_WIDTH));
	scene.blocks.push(PlaceBlock(-400+400-BLOCK_WIDTH, -250-BLOCK_WIDTH));
	scene.blocks.push(PlaceBlock(-400, -250-BLOCK_WIDTH));
	scene.blocks.push(PlaceBlock(-450, -BLOCK_WIDTH));

	scene.monsters.push(PlaceMonster(-400+50+MONSTER_WIDTH, -250-MONSTER_HEIGHT));
	scene.monsters.push(PlaceMonster(state->worldBound.x-400+MONSTER_WIDTH, -MONSTER_HEIGHT));

	scene.loadNextLevel = nullptr;
};
void buildScene1(){
	Scene &scene = state->curScene;

	scene.player.pos = {200, -PLAYER_HEIGHT};
	scene.player.vel = {0.0};
	scene.player.aura = {300, 100};
	scene.player.prop = 0;

	scene.camera.zoom = 1;

	scene.door.pos = {400, -DOOR_HEIGHT};
	scene.key.pos = {-state->worldBound.x + KEY_WIDTH, -150-KEY_HEIGHT-30};

	scene.plats.push(PlacePlatform(-400, 0, 1000, 300));
	scene.plats.push(PlacePlatform(-state->worldBound.x, -150, 550, 40));

	scene.blocks.push(PlaceBlock(-state->worldBound.x + 550 - BLOCK_WIDTH, -150 - BLOCK_HEIGHT));
	scene.blocks.push(PlaceBlock(-400+1000-BLOCK_WIDTH, 0-BLOCK_HEIGHT));

	scene.monsters.push(PlaceMonster(-state->worldBound.x + MONSTER_WIDTH, -150 - MONSTER_HEIGHT));
	scene.loadNextLevel = buildScene2;
};
