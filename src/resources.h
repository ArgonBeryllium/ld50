#pragma once
#include <SDL2/SDL_mixer.h>
#include <cumt/cumt.h>
#include <SDL2/SDL_image.h>
#include <cumt/cumt_render.h>

static inline SDL_Colour hexc(int c) { return {Uint8(c>>16), Uint8((c>>8) & 0xFF), Uint8(c & 0xFF),255}; }

inline SDL_Texture* SHEET_LOGO;
inline SDL_Colour C_BG = hexc(0x1c2638), C_DICK=hexc(0x9b222b), C_HORNS=hexc(0xf14e52), C_WALLS=hexc(0x23495d);
inline Mix_Chunk* S_SPLASH;

inline cumt::render::TextData TD_DEF_L, TD_DEF_C, TD_DEF_R, TD_JP_C;

inline SDL_Texture* loadTexture(const char* path)
{
	SDL_Texture* out = IMG_LoadTexture(shitrndr::ren, (std::string("res/")+path).c_str());
	if(!out) std::cerr << "couldn't load texture: " << IMG_GetError() << '\n';
	return out;
}
inline Mix_Chunk* loadSound(const char* path)
{
	Mix_Chunk* out = Mix_LoadWAV((std::string("res/")+path).c_str());
	if(!out) std::cerr << "couldn't load chunk: " << Mix_GetError() << '\n';
	return out;
}
inline void loadResources()
{
	using namespace cumt;
	audio::init();

	SHEET_LOGO = loadTexture("logo.png");
	S_SPLASH = loadSound("splash.wav");

	RenderData::loadFont("res/ProggyTiny.ttf", 12);
	RenderData::loadFont("res/nicomoji-plus_1.11.ttf", 12);
	RenderData::loadFont("res/m6x11.ttf", 14);
	TD_DEF_L = render::TextData{0, render::TextData::LEFT};
	TD_DEF_C = render::TextData{0, render::TextData::CENTRE};
	TD_DEF_R = render::TextData{0, render::TextData::RIGHT};
	TD_JP_C = render::TextData{1, render::TextData::CENTRE};
}
