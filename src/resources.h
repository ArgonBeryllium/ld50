#pragma once
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <cstdint>
#include <cumt/cumt.h>
#include <SDL2/SDL_image.h>
#include <cumt/cumt_render.h>
#include <fstream>
#include <codecvt>
#include <sstream>

constexpr inline SDL_Colour itoc(const uint32_t& c)
{
	return {static_cast<Uint8>(c>>16), static_cast<Uint8>(c>>8 & 0xFF), static_cast<Uint8>(c & 0xFF)};
}
constexpr inline uint32_t ctoi(const SDL_Colour& c)
{
	return (uint32_t(c.r) << 16) | (uint32_t(c.g) << 8) | (uint32_t(c.b));
}

inline SDL_Texture* SHEET_LOGO;
inline Mix_Chunk* S_SPLASH;

inline constexpr uint32_t C_BG = (0x010203), C_FG = (0xfafffd), C_GRAY = (0x8a8f8d), C_GOAL = (0xffaa11), C_HURT = (0xff2219);
inline constexpr uint32_t CS_FLAME[4] = {0xf79009, 0xf73509, 0xf79009, 0xfce4ae};
inline constexpr uint32_t CS_LEVEL[4] = {0x254259, C_GRAY, 0x3b2559, 0x397a3e};

inline TTF_Font* FONT;
constexpr float FONT_SIZE = 12, FONT_RATIO = 5./6, FONT_W = FONT_SIZE*FONT_RATIO;

inline std::wstring SPR_P, SPR_P_WAIT, SPR_SNAIL, SPR_SNAIL_WAIT;
inline std::wstring SPRS_P_WALK[2], SPRS_P_ROLL[2];
inline constexpr char FLAME_CHARS[9] = ".+*!O";
inline constexpr int FLAME_CHARS_C = 9;

inline float getLum(const SDL_Colour& c) { return float(c.r+c.g+c.b)/3/255; }
const inline wchar_t* BCHARS = L"  -+x#";
//L"·∙•○";
const int BCHARS_C = 6;

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
} // thank you LihO stackoverflow https://stackoverflow.com/questions/4775437/read-unicode-utf-8-file-into-wstring

// thank you stackoverflow https://stackoverflow.com/questions/4775437/read-unicode-utf-8-file-into-wstring
inline std::wstring loadStrSprite(const char* path)
{
	std::wifstream wif(path);
	wif.imbue(std::locale({}, new std::codecvt_utf8<wchar_t>));
	std::wstringstream wss;
	wss << wif.rdbuf();
	return wss.str();
}

inline void loadResources()
{
	using namespace cumt;
	audio::init();

	SPR_P = loadStrSprite("res/torchie2.txt");
	SPR_SNAIL = loadStrSprite("res/snail.txt");
	SPR_SNAIL_WAIT = loadStrSprite("res/snail_wait.txt");
	SPR_P_WAIT = loadStrSprite("res/p_wait.txt");

	SPRS_P_WALK[0] = loadStrSprite("res/pwalk_1.txt");
	SPRS_P_WALK[1] = loadStrSprite("res/pwalk_2.txt");
	SPRS_P_ROLL[0] = loadStrSprite("res/p_roll1.txt");
	SPRS_P_ROLL[1] = loadStrSprite("res/p_roll2.txt");

	SHEET_LOGO = loadTexture("logo.png");
	S_SPLASH = loadSound("splash.wav");

	RenderData::loadFont("res/Ac437_IBM_BIOS.ttf", FONT_SIZE);
	FONT = RenderData::fonts[0];
	TD_DEF_L = render::TextData{0, render::TextData::LEFT};
	TD_DEF_C = render::TextData{0, render::TextData::CENTRE};
	TD_DEF_R = render::TextData{0, render::TextData::RIGHT};
	TD_JP_C = render::TextData{1, render::TextData::CENTRE};
}

inline SDL_Texture* renderText(shitrndr::helpers::vec2<int> pos, const std::wstring& text, const SDL_Colour& fg = {255,255,255,255}, const SDL_Colour& bg = {0,0,0,0}, bool del = 1)
{
	SDL_Surface* s = TTF_RenderUNICODE(FONT, (const Uint16*)text.c_str(), fg, bg);
	SDL_Texture* out = SDL_CreateTextureFromSurface(shitrndr::ren, s);
	SDL_FreeSurface(s);

	int w, h;
	SDL_QueryTexture(out, 0, 0, &w, &h);
	SDL_Rect r = {pos.x, pos.y, w, h};
	shitrndr::Copy(out, r);

	if(del)
	{
		SDL_DestroyTexture(out);
		return nullptr;
	}
	return out;
}
inline SDL_Texture* renderText(shitrndr::helpers::vec2<int> pos, const wchar_t& c, const SDL_Colour& fg = {255,255,255,255}, const SDL_Colour& bg = {0,0,0,0}, bool del = 1)
{
	std::wstring s;
	s += c;
	return renderText(pos, s, fg, bg, del);
}

inline shitrndr::helpers::vec2<int> quantisePos(const shitrndr::helpers::vec2<int>& p)
{
	using namespace shitrndr;
	return (p.to<float>() / helpers::vec2<float>{FONT_W, FONT_SIZE}).to<int>();
}
inline shitrndr::helpers::vec2<int> deQuantisePos(const shitrndr::helpers::vec2<int>& p)
{
	using namespace shitrndr;
	return (p.to<float>() * helpers::vec2<float>{FONT_W, FONT_SIZE}).to<int>();
}

inline void put_char(const shitrndr::helpers::vec2<int>& p, const wchar_t& c, const uint32_t& f_col = C_FG, const uint32_t& b_col = C_BG)
{
	using namespace shitrndr;
	static std::map<wchar_t, std::map<uint64_t, SDL_Texture*>> cache;
	uint64_t ci = uint64_t(b_col) << 32 | f_col;
	SDL_Texture* t = cache[c][ci];
	if(!t) t = cache[c][ci] = renderText(p, c, itoc(f_col), itoc(b_col), 0);
	auto sp = deQuantisePos(p);
	Copy(t, {sp.x, sp.y, static_cast<int>(FONT_W), static_cast<int>(FONT_SIZE)});
}
inline void put_string(const shitrndr::helpers::vec2<int>& p, const std::wstring& s, bool a = 0, const uint32_t& f_col = C_FG, const uint32_t& b_col = C_BG)
{
	using namespace shitrndr;
	int xo = 0, yo = 0;
	for(wchar_t c : s)
	{
		xo++;
		if(c==' ') continue;
		if(c=='\n')
		{
			xo = 0;
			yo++;
			continue;
		}
		put_char(p+helpers::vec2<int>{(a?-1:1)*xo,yo}, c, f_col, b_col);
	}
}
inline void put_qstring(const shitrndr::helpers::vec2<int>& p, const std::wstring& s, bool a = 0, const uint32_t& f_col = C_FG, const uint32_t& b_col = C_BG) { put_string(quantisePos(p), s, a, f_col, b_col); }

inline static int sign(int i) { return i>0?1:i<0?-1:0; }

inline void s_line(cumt::v2i a, cumt::v2i b, wchar_t c = '#', const uint32_t& f_col = C_FG, const uint32_t& b_col = C_BG)
{
	using namespace cumt;
	v2i d = b-a;
	int sdx=sign(d.x), sdy=sign(d.y);

	int i = 0;
	put_char(b, c, f_col, b_col);
	if(!d.x)      while(a.y-b.y-sdy && i != 10000) { i++; put_char(a, c, f_col, b_col); a.y+=sdy; }
	else if(!d.y) while(a.x-b.x-sdx && i != 10000) { i++; put_char(a, c, f_col, b_col); a.x+=sdx; }
	else while((a.x != b.x || a.y != b.y) && i != 10000)
	{
		i++;
		float dx = b.x-a.x, dy = b.y-a.y;
		put_char(a, c, f_col, b_col);

		if(dx/d.x > dy/d.y)
			a.x+=sdx;
		else
			a.y+=sdy;
	}
	if(i==10000) std::cerr << "s_line fail\n";
}
inline void s_qline(cumt::v2i a, cumt::v2i b, wchar_t c = '#', const uint32_t& f_col = C_FG, const uint32_t& b_col = C_BG)
{
	s_line(quantisePos(a), quantisePos(b), c, f_col, b_col);
}
inline void s_dcircle(const cumt::v2i& p, int r, wchar_t c = '#', const uint32_t& f_col = C_FG, const uint32_t& b_col = C_BG)
{
	float l = 2*M_PIf32*r;
	for(int i = 1; i < l; i++)
	{
		float a = i/l*M_PIf32*2;
		put_char({(int)(std::cos(a)*r)+p.x, (int)(std::sin(a)*r+p.y)}, c, f_col, b_col);
	}
}
inline void s_fcircle(const cumt::v2i& p, int r, wchar_t c = '#', const uint32_t& f_col = C_FG, const uint32_t& b_col = C_BG)
{
	float l = 2*M_PIf32*r;
	for(int i = 1; i < l; i++)
	{
		float a = i/l*M_PIf32*2;
		s_line(p, {(int)(std::cos(a)*r)+p.x, (int)(std::sin(a)*r+p.y)}, c, f_col, b_col);
	}
}
inline void s_qdcircle(const cumt::v2i& p, int r, wchar_t c = '#', const uint32_t& f_col = C_FG, const uint32_t& b_col = C_BG) { s_dcircle(quantisePos(p), r, c, f_col, b_col); }
inline void s_qfcircle(const cumt::v2i& p, int r, wchar_t c = '#', const uint32_t& f_col = C_FG, const uint32_t& b_col = C_BG) { s_fcircle(quantisePos(p), r, c, f_col, b_col); }
