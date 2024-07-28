#include "game.hh"
#include "ds.cc"
#include <float.h>
#include "animation.cc"

#define MONSTER_WIDTH  100
#define MONSTER_HEIGHT 100
#define BLOCK_WIDTH    20
#define BLOCK_HEIGHT   20
#define PLAYER_HEIGHT  100
#define PLAYER_WIDTH   80
#define MONSTER_VEL_X  100
#define PLAYER_FOOT_HEIGHT 5
#define DOOR_WIDTH     70
#define DOOR_HEIGHT    80
#define KEY_WIDTH      10
#define KEY_HEIGHT     20

#define GET_MONSTERS(swapss) ((Monster*)((char*)swapss + sizeof(SwapScreenShot)))

enum class PlayerProp{
	IS_GROUND,
	HAS_KEY,
	IS_DEAD,
};
enum class SceneProp{
	NO_KEY,
};

struct Player{
	Vector2 pos;
	Vector2 vel;
	Vector2 aura;
	u32     prop;
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
struct Door{
	Vector2 pos;
};
struct Key{
	Vector2 pos;
};
struct SwapScreenShot{
	Player player;
	SwapScreenShot *prev;
	// NOTE: Monsters are saved in the consecutive memory block
};
void freeSwapScreenShotChain(SwapScreenShot *tail){
	while(tail){
		SwapScreenShot *prev = tail->prev;
		afree(tail);
		tail = prev;
	};
};
struct Scene{
	Player player;
	Camera2D camera;
	Door   door;
	Key    key;
	DynamicArray<Block> blocks;
	DynamicArray<Platform> plats;
	DynamicArray<Monster> monsters;
	void (*loadNextLevel)();
	u32 prop;
	SwapScreenShot *cur;
	SwapScreenShot *tail;

	void init(){
		cur = nullptr;
		tail = nullptr;
		prop = 0;
		player.prop = 0;
		plats.init(2);
		blocks.init(2);
		monsters.init(1);
		camera.target = {0.0};
		camera.offset = { GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
		camera.rotation = 0.0f;
		camera.zoom = 1.0f;
	};
	void uninit(){
		freeSwapScreenShotChain(tail);
		plats.uninit();
		blocks.uninit();
		monsters.uninit();
	};
};
struct GlobalState{
	Scene curScene;
	PackageManager pm;
	Vector2 worldBound; // NOTE: .x contains right edge and .y contains left edge world pos
	SpriteAnimation sa;
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

	scene.player.pos = {200, -100};
	scene.player.vel = {0.0};
	scene.player.aura = {300, 100};
	scene.player.prop = 0;

	scene.camera.zoom = 1;

	scene.door.pos = {400, -100};
	scene.key.pos = {-state->worldBound.x + KEY_WIDTH, -150-KEY_HEIGHT};

	scene.monsters.push(PlaceMonster(-state->worldBound.x + MONSTER_WIDTH, -150 - MONSTER_HEIGHT));

	scene.plats.push(PlacePlatform(-400, 0, 1000, 300));
	scene.plats.push(PlacePlatform(-state->worldBound.x, -150, 550, 40));

	scene.blocks.push(PlaceBlock(-state->worldBound.x + 550 - BLOCK_WIDTH, -150 - BLOCK_HEIGHT));
	scene.blocks.push(PlaceBlock(-400+1000-BLOCK_WIDTH, 0-BLOCK_HEIGHT));
};
void resetCurScene(){
	Scene &scene = state->curScene;
	scene.plats.count = 0;
	scene.blocks.count = 0;
	scene.monsters.count = 0;
	scene.loadNextLevel = nullptr;
	scene.player = {0};
	scene.prop = 0;
	freeSwapScreenShotChain(scene.tail);
	scene.cur = nullptr;
	scene.tail = nullptr;
};
void undo(){
	Scene &scene = state->curScene;
	SwapScreenShot *ss = scene.cur;
	if(ss == nullptr) return;
	scene.player = ss->player;
	Monster *mons = GET_MONSTERS(ss);
	memcpy(scene.monsters.mem, mons, scene.monsters.count * sizeof(Monster));
	if(ss->prev) scene.cur = ss->prev;
};
void takeSwapScreenShot(){
	Scene &scene = state->curScene;
	SwapScreenShot *ss = (SwapScreenShot*)alloc(sizeof(SwapScreenShot) + sizeof(Monster)*scene.monsters.count);
	ss->player = scene.player;
	ss->prev = scene.cur;
	scene.tail = ss;
	scene.cur = ss;
	Monster *mons = GET_MONSTERS(ss);
	memcpy(mons, scene.monsters.mem, scene.monsters.count * sizeof(Monster));
	scene.cur = ss;
};
EXPORT void gameInit(void *gameMem){
	state->curScene.init();
	state->worldBound = GetScreenToWorld2D({(float)GetScreenWidth(), (float)GetScreenHeight()}, state->curScene.camera);
	state->pm.init("TODO:");
	buildScene1();	
	takeSwapScreenShot();
	s32 size;
	void *spriteSheet = state->pm.getFile("idle.png", size);
	Image img = LoadImageFromMemory(".png", (const unsigned char*)spriteSheet, size);
	Texture2D text = LoadTextureFromImage(img);
	state->sa.init(text, 20, 0.08);
};
EXPORT void gameUpdate(f32 dt){
	Scene &scene = state->curScene;
	Player &player = scene.player;
	if(IsKeyPressed(KEY_U)) undo();
	if(IS_BIT(player.prop, PlayerProp::IS_DEAD)){
		BeginDrawing();
		BeginMode2D(scene.camera);
		DrawText("UNDO", -10, -10, 100, WHITE);
		EndMode2D();
		EndDrawing();
		return;
	};
	updateAnimation(state->sa, dt);
	DynamicArray<Monster> &monsters = scene.monsters;
	DynamicArray<Block>   &blocks   = scene.blocks;
	DynamicArray<Platform> &plats   = scene.plats;
	if(IsKeyDown(KEY_SPACE) && IS_BIT(player.prop, PlayerProp::IS_GROUND)) player.vel.y = -800;
	if(IsKeyDown(KEY_D)) player.vel.x = 200;
	if(IsKeyDown(KEY_A)) player.vel.x = -200;
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
	if(canSwap && IsKeyPressed(KEY_E)){
		takeSwapScreenShot();
		Vector2 temp = closestM->pos;
		closestM->pos = player.pos;
		player.pos = temp;
	};
	BeginDrawing();
	ClearBackground(PINK);
	DrawFPS(0,0);
	BeginMode2D(scene.camera);
	DrawTexturePro(state->sa.texture, getFrame(state->sa), {player.pos.x, player.pos.y, PLAYER_WIDTH, PLAYER_HEIGHT}, {0.0, 0.0}, 0.0, WHITE);
	for(u32 x=0; x<plats.count; x++){
		DrawRectangleRec(plats[x].rec, BLUE);
	};
	for(u32 x=0; x<blocks.count; x++){
		DrawRectangleV(blocks[x].pos, {BLOCK_WIDTH, BLOCK_HEIGHT}, PURPLE);
	};
	for(u32 x=0; x<monsters.count; x++){
		Monster &monster = monsters[x];
		DrawRectangle(monster.pos.x, monster.pos.y, MONSTER_WIDTH, MONSTER_HEIGHT, ((closestM == &monster) && canSwap)?GREEN:WHITE);
	};
	DrawRectangleRec(auraRec, {255, 0, 255, 50});
	if(!IS_BIT(player.prop, PlayerProp::HAS_KEY) && !IS_BIT(scene.prop, SceneProp::NO_KEY)) DrawRectangleV(scene.key.pos, {KEY_WIDTH, KEY_HEIGHT}, BROWN);
	DrawRectangleV(scene.door.pos, {DOOR_WIDTH, DOOR_HEIGHT}, BROWN);
	EndMode2D();
	EndDrawing();
};
EXPORT void gamePhyUpdate(){
	Scene &scene = state->curScene;
	Player &player = scene.player;
	if(IS_BIT(player.prop, PlayerProp::IS_DEAD)) return;
	DynamicArray<Monster> &monsters = scene.monsters;
	DynamicArray<Block>   &blocks   = scene.blocks;
	DynamicArray<Platform> &plats   = scene.plats;
	Rectangle playerRec;
	playerRec.x = player.pos.x;
	playerRec.y = player.pos.y;
	playerRec.width = PLAYER_WIDTH;
	playerRec.height = PLAYER_HEIGHT;
	Rectangle doorRec;
	doorRec.x = scene.door.pos.x;
	doorRec.y = scene.door.pos.y;
	doorRec.width = DOOR_WIDTH;
	doorRec.height = DOOR_HEIGHT;
	if(CheckCollisionRecs(playerRec, doorRec)){
		if(player.vel.x > 0){
			player.vel.x = 0;
			player.pos.x = doorRec.x - PLAYER_WIDTH;
		};
		if(player.vel.x < 0){
			player.vel.x = 0;
			player.pos.x = doorRec.x + DOOR_WIDTH;
		};
		if((IS_BIT(player.prop, PlayerProp::HAS_KEY) || IS_BIT(scene.prop, SceneProp::NO_KEY))){
			clog("changing level");
		};
	};
	Rectangle keyRec;
	keyRec.x = scene.key.pos.x;
	keyRec.y = scene.key.pos.y;
	keyRec.width = KEY_WIDTH;
	keyRec.height = KEY_HEIGHT;
	if(CheckCollisionRecs(playerRec, keyRec)) SET_BIT(player.prop, PlayerProp::HAS_KEY);
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
			SET_BIT(player.prop, PlayerProp::IS_GROUND);
		};
	}else{
		CLEAR_BIT(player.prop, PlayerProp::IS_GROUND);
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
		if(CheckCollisionRecs(monsterRect, playerRec)){
			SET_BIT(player.prop, PlayerProp::IS_DEAD);
			return;
		};	
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
		}else monster.vel.y += 50;
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
		if(monster.pos.x+MONSTER_WIDTH >= state->worldBound.x || (monster.pos.x <= -state->worldBound.x)){monster.vel.x *= -1;};
		monster.pos.x += monster.vel.x * dt;
		monster.pos.y += monster.vel.y * dt;
	};
};
EXPORT void gameUninit(){
	state->curScene.uninit();
	state->pm.uninit();
	clog("Bye from game 1");
};
