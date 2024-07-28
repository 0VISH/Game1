// NOTE: This file assumes that your sprite sheet is horizontal

struct SpriteAnimation{
    Texture2D texture;
    u32 frames;
    f32 frameTimeLen;

    u32 curFrame;
    f32 frameTimer;

    void init(Texture2D tex, u32 frameCount, f32 frameTimeLength){
	texture = tex;
	frames = frameCount;
	frameTimeLen = frameTimeLength;

	curFrame = 0;
	frameTimer = frameTimeLength;
    };
    void reset(){
	curFrame = 0;
	frameTimer = frameTimeLen;
    };
    void uninit(){
	UnloadTexture(texture);
    };
};

void updateAnimation(SpriteAnimation *sa, f32 dt){
    sa->frameTimer -= dt;
    if(sa->frameTimer <= 0.0f){
	sa->curFrame += 1;
	if(sa->curFrame == sa->frames) sa->curFrame = 0;
	sa->frameTimer = sa->frameTimeLen + sa->frameTimer;
    };
};

Rectangle getFrame(SpriteAnimation *sa){
    const f32 textWidth = sa->texture.width/(f32)sa->frames;
    return {(f32)(sa->curFrame * textWidth), 0.0f, (f32)textWidth, (f32)sa->texture.height};
};
