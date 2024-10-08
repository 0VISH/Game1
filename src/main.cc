#include "game.hh"
#include "ds.cc"
#include <float.h>
#include "animation.cc"

#define MONSTER_WIDTH  100
#define MONSTER_HEIGHT 100
#define BLOCK_WIDTH    20
#define BLOCK_HEIGHT   20
#define PLAYER_HEIGHT  120
#define PLAYER_WIDTH   80
#define MONSTER_VEL_X  50
#define PLAYER_FOOT_HEIGHT 5
#define DOOR_WIDTH     80
#define DOOR_HEIGHT    125
#define KEY_WIDTH      30
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
	s16     dir;
	SpriteAnimation idle;
	SpriteAnimation walk;
	SpriteAnimation *cur;
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
	Vector2 pos;
	u32 prop;
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
		player.dir = 1;
		player.cur = &player.idle;
		plats.init(5);
		blocks.init(5);
		monsters.init(5);
		camera.target = {0.0};
		camera.offset = { GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
		camera.rotation = 0.0f;
		camera.zoom = 1.0f;
	};
	void uninit(){
		freeSwapScreenShotChain(tail);
		player.idle.uninit();
		player.walk.uninit();
		plats.uninit();
		blocks.uninit();
		monsters.uninit();
	};
};
struct GlobalState{
	Scene curScene;
	PackageManager pm;
	Vector2 worldBound; // NOTE: .x contains right edge and .y contains left edge world pos
	SpriteAnimation gmonster;
	Texture2D background;
};
GlobalState *state;
void gameReload(void *gameMem){
	state = (GlobalState*)gameMem;
};

#include "scene.cc"

void resetCurScene(){
	Scene &scene = state->curScene;
	scene.plats.count = 0;
	scene.blocks.count = 0;
	scene.monsters.count = 0;
	scene.prop = 0;
	scene.player.idle.reset();
	scene.player.walk.reset();
	scene.player.cur = &scene.player.idle;
	scene.player.prop = 0;
	state->gmonster.reset();
	freeSwapScreenShotChain(scene.tail);
	scene.cur = nullptr;
	scene.tail = nullptr;
};
void undo(){
	Scene &scene = state->curScene;
	SwapScreenShot *ss = scene.cur;
	Player &player = scene.player;
	if(ss == nullptr) return;
	player.pos = ss->pos;
	player.prop = ss->prop;
	player.vel = {0.0};
	player.cur->reset();
	player.cur = &scene.player.idle;
	Monster *mons = GET_MONSTERS(ss);
	memcpy(scene.monsters.mem, mons, scene.monsters.count * sizeof(Monster));
	if(ss->prev) scene.cur = ss->prev;
};
void takeSwapScreenShot(){
	Scene &scene = state->curScene;
	SwapScreenShot *ss = (SwapScreenShot*)alloc(sizeof(SwapScreenShot) + sizeof(Monster)*scene.monsters.count);
	ss->pos = scene.player.pos;
	ss->prop = scene.player.prop;
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
	state->pm.init("assets.pkg");
	buildScene1();	
	takeSwapScreenShot();
	s32 size;
	void *spriteSheet = state->pm.getFile("assets\\idle.png", size);
	clog("%lld\n", size);
	Image img = LoadImageFromMemory(".png", (const unsigned char*)spriteSheet, size);
	Texture2D text = LoadTextureFromImage(img);
	state->curScene.player.idle.init(text, 20, 0.08);
	spriteSheet = state->pm.getFile("assets\\walk.png", size);
	img = LoadImageFromMemory(".png", (const unsigned char*)spriteSheet, size);
	text = LoadTextureFromImage(img);
	UnloadImage(img);
	state->curScene.player.walk.init(text, 20, 0.04);
	spriteSheet = state->pm.getFile("assets\\slimeGreen.png", size);
	img = LoadImageFromMemory(".png", (const unsigned char*)spriteSheet, size);
	text = LoadTextureFromImage(img);
	UnloadImage(img);
	state->gmonster.init(text, 30, 0.05);
	spriteSheet = state->pm.getFile("assets\\background.png", size);
	img = LoadImageFromMemory(".png", (const unsigned char*)spriteSheet, size);
	text = LoadTextureFromImage(img);
	UnloadImage(img);
	state->background = text;
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
	DynamicArray<Monster> &monsters = scene.monsters;
	DynamicArray<Block>   &blocks   = scene.blocks;
	DynamicArray<Platform> &plats   = scene.plats;
	bool keyDown = false;
	if(IsKeyDown(KEY_SPACE) && IS_BIT(player.prop, PlayerProp::IS_GROUND)){
		keyDown = true;
		player.vel.y = -800;
	}else if(IsKeyDown(KEY_D)){
		keyDown = true;
		player.dir = 1;
		player.vel.x = 200;
		if(player.cur != &player.walk) player.cur->reset();
		player.cur = &player.walk;
	}else if(IsKeyDown(KEY_A)){
		keyDown = true;
		player.dir = -1;
		player.vel.x = -200;
		if(player.cur != &player.walk) player.cur->reset();
		player.cur = &player.walk;
	};
	if(!keyDown){
		player.cur = &player.idle;
		player.vel.x = 0.0;
	};
	updateAnimation(player.cur, dt);
	updateAnimation(&state->gmonster, dt);
	f64 closest = DBL_MAX;
	Monster *closestM = nullptr;
	for(u32 x=0; x<monsters.count; x++){
		Monster &monster = monsters[x];
		f64 distance = sqrt((player.pos.x-monster.pos.x)*(player.pos.x-monster.pos.x) + (player.pos.y-monster.pos.y)*(player.pos.y-monster.pos.y));
		if(distance < closest){
			closest = distance;
			closestM = &monster;
		};
	};
	b32 canSwap = false;
	Rectangle auraRec;
	auraRec.x = player.pos.x-player.aura.x;
	auraRec.y = player.pos.y-player.aura.y;
	auraRec.width = 100 + player.aura.x*2;
	auraRec.height = 100 + player.aura.y*2;
	if(closestM){
		Rectangle monsterRec;
		monsterRec.x = closestM->pos.x;
		monsterRec.y = closestM->pos.y;
		monsterRec.width = MONSTER_WIDTH;
		monsterRec.height = MONSTER_HEIGHT;
		if(CheckCollisionRecs(auraRec, monsterRec) && IsKeyPressed(KEY_E)){
			takeSwapScreenShot();
			Vector2 temp = closestM->pos;
			closestM->pos = player.pos;
			closestM->pos.y += PLAYER_HEIGHT - MONSTER_HEIGHT;
			player.pos = temp;
			player.pos.y += MONSTER_HEIGHT - PLAYER_HEIGHT;
			canSwap = true;
		};
	};
	BeginDrawing();
	ClearBackground(PINK);
	DrawTexturePro(state->background, getFrame(state->background, 0, 3), {0.0, 0.0, (f32)GetScreenWidth(), (f32)GetScreenHeight()}, {0.0, 0.0}, 0.0, WHITE);
	DrawTexturePro(state->background, getFrame(state->background, 1, 3), {0.0, 0.0, (f32)GetScreenWidth(), (f32)GetScreenHeight()}, {0.0, 0.0}, 0.0, WHITE);
	DrawFPS(0,0);
	BeginMode2D(scene.camera);
	DrawRectangleV(scene.door.pos, {DOOR_WIDTH, DOOR_HEIGHT}, BROWN);
	Rectangle r = getAnimationFrame(player.cur);
	r.width *= player.dir;
	DrawTexturePro(player.cur->texture, r, {player.pos.x, player.pos.y, PLAYER_WIDTH, PLAYER_HEIGHT}, {0.0, 0.0}, 0.0, WHITE);
	for(u32 x=0; x<plats.count; x++){
		DrawRectangleRec(plats[x].rec, BLUE);
	};
	for(u32 x=0; x<blocks.count; x++){
		DrawRectangleV(blocks[x].pos, {BLOCK_WIDTH, BLOCK_HEIGHT}, PURPLE);
	};
	for(u32 x=0; x<monsters.count; x++){
		Monster &monster = monsters[x];
		DrawTexturePro(state->gmonster.texture, getAnimationFrame(&state->gmonster), {monster.pos.x, monster.pos.y, MONSTER_WIDTH, MONSTER_HEIGHT}, {0.0, 0.0}, 0.0, WHITE);
	};
	DrawRectangleRec(auraRec, {255, 0, 255, 50});
	if(!IS_BIT(player.prop, PlayerProp::HAS_KEY) && !IS_BIT(scene.prop, SceneProp::NO_KEY)) DrawRectangleV(scene.key.pos, {KEY_WIDTH, KEY_HEIGHT}, {255, 215, 0, 255});
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
		if(player.vel.x > 0 && playerRec.x < doorRec.x){
			player.vel.x = 0;
			player.pos.x = doorRec.x - PLAYER_WIDTH;
		};
		if(player.vel.x < 0 && playerRec.x > doorRec.x){
			player.vel.x = 0;
			player.pos.x = doorRec.x + DOOR_WIDTH;
		};
		if((IS_BIT(player.prop, PlayerProp::HAS_KEY) || IS_BIT(scene.prop, SceneProp::NO_KEY))){
			if(scene.loadNextLevel){
				resetCurScene();
				scene.loadNextLevel();
				return;
			};
			exitOKC();
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
	state->gmonster.uninit();
	UnloadTexture(state->background);
	clog("Bye from game 1");
};
