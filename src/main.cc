#include "game.hh"
#include "ds.cc"
#include <float.h>

#define MONSTER_WIDTH  100
#define MONSTER_HEIGHT 100
#define BLOCK_WIDTH    20
#define BLOCK_HEIGHT   20
#define PLAYER_HEIGHT  100
#define PLAYER_WIDTH   100
#define MONSTER_VEL_X  100
#define PLAYER_FOOT_HEIGHT 5

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
struct Scene{
	Player player;
	Camera2D camera;
	Array<Block> blocks;
	Array<Platform> plats;
	Array<Monster> monsters;
};
struct GlobalState{
	Scene curScene;
	u32   screenX;
	u32   screenY;
};
GlobalState *state;
void gameReload(void *gameMem){
	state = (GlobalState*)gameMem;
};

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
void buildScene1(){
	Scene &scene = state->curScene;

	scene.plats.init(2);
	scene.blocks.init(2);
	scene.monsters.init(1);

	scene.camera.target = {0.0};
	scene.camera.offset = { 1800/2.0f, 900/2.0f };
	scene.camera.rotation = 0.0f;
	scene.camera.zoom = 1.0f;

	scene.player.pos = {200, -100};
	scene.player.vel = {0.0};
	scene.player.aura = {300, 100};
	scene.player.isGround = true;

	scene.monsters.push(PlaceMonster(20, -50 - MONSTER_HEIGHT));

	scene.plats.push(PlacePlatform(100, 0, 500, 300));
	scene.plats.push(PlacePlatform(-10, -50, 200, 10));

	scene.blocks.push(PlaceBlock(-10, -50 - BLOCK_HEIGHT));
	scene.blocks.push(PlaceBlock(-10 + 200 - BLOCK_WIDTH, -50 - BLOCK_HEIGHT));
};
void destroyScene1(){
	Scene &scene = state->curScene;
	scene.plats.uninit();
	scene.blocks.uninit();
	scene.monsters.uninit();
};

EXPORT void gameInit(void *gameMem){
	buildScene1();	
};
EXPORT void gameUpdate(f32 dt){
	Scene &scene = state->curScene;
	Player &player = scene.player;
	Array<Monster> &monsters = scene.monsters;
	Array<Block>   &blocks   = scene.blocks;
	Array<Platform> &plats   = scene.plats;
	if(IsKeyDown(KEY_SPACE) && player.isGround){
		player.vel.y = -800;
	};
	if(IsKeyDown(KEY_D)){
		player.vel.x = 200;
	};
	if(IsKeyDown(KEY_A)){
		player.vel.x = -200;
	};
	f64 closest = DBL_MAX;
	Monster *closestM = nullptr;
	for(u32 x=0; x<monsters.len; x++){
		Monster &monster = monsters[x];
		f64 distance = sqrt((player.pos.x*player.pos.x) + (monster.pos.x*monster.pos.x));
		if(distance < closest){
			closest = distance;
			closestM = &monster;
		};
	};
	b32 canSwap = false;
	Rectangle monsterRec;
	monsterRec.x = closestM->pos.x;
	monsterRec.y = closestM->pos.y;
	monsterRec.width = MONSTER_WIDTH;
	monsterRec.height = MONSTER_HEIGHT;
	Rectangle auraRec;
	auraRec.x = player.pos.x-player.aura.x;
	auraRec.y = player.pos.y-player.aura.y;
	auraRec.width = 100 + player.aura.x*2;
	auraRec.height = 100 + player.aura.y*2;
	if(CheckCollisionRecs(auraRec, monsterRec)) canSwap=true;
	if(canSwap && IsKeyPressed(KEY_U)){
		Vector2 temp = closestM->pos;
		closestM->pos = player.pos;
		player.pos = temp;
	};
	BeginDrawing();
	ClearBackground(BLACK);
	DrawFPS(0,0);
	BeginMode2D(scene.camera);
	DrawRectangleV(player.pos, {PLAYER_WIDTH, PLAYER_HEIGHT-PLAYER_FOOT_HEIGHT}, RED);
	DrawRectangle(player.pos.x, player.pos.y + PLAYER_HEIGHT - PLAYER_FOOT_HEIGHT, PLAYER_WIDTH, PLAYER_FOOT_HEIGHT, YELLOW);
	for(u32 x=0; x<plats.count; x++){
		DrawRectangleRec(plats[x].rec, BLUE);
	};
	for(u32 x=0; x<blocks.count; x++){
		DrawRectangleV(blocks[x].pos, {BLOCK_WIDTH, BLOCK_HEIGHT}, PURPLE);
	};
	for(u32 x=0; x<monsters.count; x++){
		Monster &monster = monsters[x];
		DrawRectangle(monster.pos.x, monster.pos.y, MONSTER_WIDTH, MONSTER_HEIGHT, (closestM == &monster)?GREEN:WHITE);
	};
	DrawRectangleRec(auraRec, {255, 0, 255, 50});
	EndMode2D();
	EndDrawing();
};
EXPORT void gamePhyUpdate(){
	Scene &scene = state->curScene;
	Player &player = scene.player;
	Array<Monster> &monsters = scene.monsters;
	Array<Block>   &blocks   = scene.blocks;
	Array<Platform> &plats   = scene.plats;
	Rectangle playerFootRec;
	playerFootRec.x = player.pos.x;
	playerFootRec.y = player.pos.y + PLAYER_HEIGHT - PLAYER_FOOT_HEIGHT;
	playerFootRec.width = PLAYER_WIDTH;
	playerFootRec.height = PLAYER_FOOT_HEIGHT;
	const f32 dt = (f32)1/(f32)60;
	Platform *col = nullptr;
	for(u32 x=0; x<plats.count; x++){
		Platform &plat = plats[x];
		if(CheckCollisionRecs(plat.rec, playerFootRec)){
			col = &plat;
			break;
		};
	};
	if(col){
		if(player.vel.y > 0){
			player.vel.y = 0;
			player.pos.y = col->rec.y - PLAYER_HEIGHT;
			player.isGround = true;
		};
	}else{
		player.isGround = false;
		player.vel.y += 50;
	};
	player.pos.x += player.vel.x * dt;
	player.pos.y += player.vel.y * dt;
	player.vel.x = 0.0;

	for(u32 i=0; i<monsters.count; i++){
		col = nullptr;
		Monster &monster = monsters[i];
		Rectangle monsterRect;
		monsterRect.x = monster.pos.x;
		monsterRect.y = monster.pos.y;
		monsterRect.height = MONSTER_HEIGHT;
		monsterRect.width = MONSTER_WIDTH;
		for(u32 x=0; x<plats.count; x++){
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
		for(u32 x=0; x<blocks.count; x++){
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
				monster.pos.x = bcol->pos.x - MONSTER_WIDTH;
			}else if(monster.vel.x < 0){
				monster.vel.x *= -1;
				monster.pos.x = bcol->pos.x + BLOCK_WIDTH;
			};
		};
		monster.pos.x += monster.vel.x * dt;
		monster.pos.y += monster.vel.y * dt;
	};
};
EXPORT void gameUninit(){
	destroyScene1();
	clog("Bye from game 1");
};
