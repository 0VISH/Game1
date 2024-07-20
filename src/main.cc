#include "game.hh"

#define MONSTER_WIDTH  100
#define MONSTER_HEIGHT 100
#define BLOCK_WIDTH    20
#define BLOCK_HEIGHT   20

void gameReload(void *gameMem, u64 memSize){
	mem::reload(gameMem, memSize);
};
struct Player{
	Vector2 pos;
	Vector2 vel;
	Vector2 aura;
	b32     isGround;
};
struct Platform{
	Rectangle rec;
};
struct Monster{
	Vector2 pos;
	Vector2 vel;
};
struct Block{
	Vector2 pos;
};
Camera2D camera;
Player   player;
Platform plats[2];
Block    blocks[2];
Monster  monster;
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
	blk.pos.x = x;
	blk.pos.y = y;
	return blk;
};
EXPORT void gameInit(void *gameMem, u64 memSize){
	camera.target = {0.0};
	camera.offset = { 1800/2.0f, 900/2.0f };
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;

	player.pos = {200, -100};
	player.vel = {0.0};
	player.aura = {300, 100};
	player.isGround = true;
	
	monster.pos = {20, -50 - MONSTER_HEIGHT};
	monster.vel = {90.0, 0.0};

	plats[0] = PlacePlatform(100, 0, 500, 300);
	plats[1] = PlacePlatform(-10, -50, 200, 10);

	blocks[0] = PlaceBlock(-10, -50 - BLOCK_HEIGHT);
	blocks[1] = PlaceBlock(-10 + 200 - BLOCK_WIDTH, -50 - BLOCK_HEIGHT);
};
EXPORT void gameUpdate(f32 dt){
	if(IsKeyDown(KEY_SPACE) && player.isGround){
		player.vel.y = -800;
	};
	if(IsKeyDown(KEY_D)){
		player.vel.x = 200;
	};
	if(IsKeyDown(KEY_A)){
		player.vel.x = -200;
	};
	//f64 distance = sqrt((player.pos.x*player.pos.x) + (monster.pos.x*monster.pos.x));
	b32 canSwap = false;
	Rectangle auraRec;
	auraRec.x = player.pos.x-player.aura.x;
	auraRec.y = player.pos.y-player.aura.y;
	auraRec.width = 100 + player.aura.x*2;
	auraRec.height = 100 + player.aura.y*2;
	Rectangle monsterRec;
	monsterRec.x = monster.pos.x;
	monsterRec.y = monster.pos.y;
	monsterRec.width = MONSTER_WIDTH;
	monsterRec.height = MONSTER_HEIGHT;
	if(CheckCollisionRecs(auraRec, monsterRec)) canSwap=true;
	if(canSwap && IsKeyPressed(KEY_R)){
		Vector2 temp = monster.pos;
		monster.pos = player.pos;
		player.pos = temp;
	};
	BeginDrawing();
	ClearBackground(BLACK);
	DrawFPS(0,0);
	BeginMode2D(camera);
	DrawRectangleV(player.pos, {100, 100}, RED);
	for(u32 x=0; x<ARRAY_LENGTH(plats); x++){
		DrawRectangleRec(plats[x].rec, BLUE);
	};
	for(u32 x=0; x<ARRAY_LENGTH(blocks); x++){
		DrawRectangleV(blocks[x].pos, {BLOCK_WIDTH, BLOCK_HEIGHT}, PURPLE);
	};
	DrawRectangleRec(monsterRec, (canSwap)?GREEN:WHITE);
	DrawRectangleRec(auraRec, {255, 0, 255, 50});
	EndMode2D();
	EndDrawing();
};
EXPORT void gamePhyUpdate(){
	Rectangle playerRec;
	playerRec.x = player.pos.x;
	playerRec.y = player.pos.y;
	playerRec.width = 100;
	playerRec.height = 100;
	const f32 dt = (f32)1/(f32)60;
	Platform *col = nullptr;
	for(u32 x=0; x<ARRAY_LENGTH(plats); x++){
		Platform &plat = plats[x];
		if(CheckCollisionRecs(plat.rec, playerRec)){
			col = &plat;
			break;
		};
	};
	if(col){
		if(player.vel.y > 0){
			player.vel.y = 0;
			player.pos.y = col->rec.y - 100;
			player.isGround = true;
		};
	}else player.isGround = false;
	col = nullptr;
	Rectangle monsterRect;
	monsterRect.x = monster.pos.x;
	monsterRect.y = monster.pos.y;
	monsterRect.height = MONSTER_HEIGHT;
	monsterRect.width = MONSTER_WIDTH;
	for(u32 x=0; x<ARRAY_LENGTH(plats); x++){
		Platform &plat = plats[x];
		if(CheckCollisionRecs(plat.rec, monsterRect)){
			col = &plat;
			break;
		};
	};
	if(col){
		if(monster.vel.y > 0){
			monster.vel.y = 0;
			monster.pos.y = col->rec.y - MONSTER_HEIGHT;
		};
	}else{
		monster.vel.y += 50;
	};
	Block *bcol = nullptr;
	for(u32 x=0; x<ARRAY_LENGTH(blocks); x++){
		Block &blk = blocks[x];
		Rectangle blockRect;
		blockRect.x = blk.pos.x;
		blockRect.y = blk.pos.y;
		blockRect.width = BLOCK_WIDTH;
		blockRect.height = BLOCK_HEIGHT;
		if(CheckCollisionRecs(blockRect, monsterRect)){
			bcol = &blk;
			break;
		};
	};
	if(bcol){
		if(monster.vel.x > 0){
			monster.vel.x *= -1;
		}else if(monster.vel.x < 0){
			monster.vel.x *= -1;
		};
	};
	monster.pos.x += monster.vel.x * dt;
	monster.pos.y += monster.vel.y * dt;
	if(!player.isGround){
		player.vel.y += 50;
	};
	player.pos.x += player.vel.x * dt;
	player.pos.y += player.vel.y * dt;
	player.vel.x = 0.0;
};
EXPORT void gameUninit(){
	clog("Bye from game 1");
};
