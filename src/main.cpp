#include <cstdint>
#include <cumt/cumt.h>
#include <shitrndr.h>
#include "creaturas.h"
#include "resources.h"
#include "scenes.h"
#include "expirables.h"

using namespace cumt;

void gameStart();
void gameLoop();
void gameKeyDown(const SDL_Keycode& key);
void renderSplash();

int CUMT_MULP_MAIN()
{
	InitParams ip;
	ip.sr_ps = 3;

	quickInit(960, 720, ip);
	onLoop = &gameLoop;
	onKey = &gameKeyDown;
	gameStart();
	shitrndr::onKeyUp = [](const SDL_Keycode& key) { Scene::getActive()->onKeyUp(key); };
	shitrndr::onKeyHeld = [](const SDL_Keycode& key) { Scene::getActive()->onKeyHeld(key); };
	shitrndr::onMBDown = [](const uint8_t& b) { Scene::getActive()->onMB(b); };
	shitrndr::onMBUp = [](const uint8_t& b) { Scene::getActive()->onMBUp(b); };
	start();
}

using namespace shitrndr;

struct S_A : Scene
{
	void load() override
	{
		bg_col = {0,0,0,255};
		set.instantiate(new LaCreatura({0,0}, {1,1}, 2));
		Thing2D::view_scale = .2;
	}
	void loop() override
	{
		render::pattern::checkerBoard(0, 30, 30);
		Scene::loop();
	}
	void onKey(SDL_Keycode key) override
	{
		for(auto p : set.things_id)
			if(LaCreatura* c = dynamic_cast<LaCreatura*>(p.second))
				c->onKey(key);
	}
	void onMB(uint8_t b) override
	{
		for(auto p : set.things_id)
			if(LaCreatura* c = dynamic_cast<LaCreatura*>(p.second))
				c->onMB(b);
	}
};
struct S_Splash : Scene
{
	float del = 2;
	void transOut(float t) override
	{
		SetColour({0,0,0,Uint8(std::min(1.f,t)*255)});
		SDL_RenderFillRect(ren, 0);
	}
	void transIn(float t) override { transOut(1-t); }

	void loop() override
	{
		static bool played = 0;
		if(FD::time>2 && !played)
		{
			audio::play(S_SPLASH, .3);
			played = 1;
		}

		bg_col = {0,0,0,255};
		v2i p = WindowProps::getSize()/2;
		int s = 100;
		int f = std::max(0,std::min(int((FD::time-del)*30), 28));
		Copy(SHEET_LOGO, {540*f,0,540,540}, {p.x-s/2, p.y-s/2, s, s});

		render::text(p+v2i{0,s/2}, "a game by arbe", TD_DEF_C);
		//render::text(p+v2i{0,s/2+12}, "アルビのゲーム", TD_JP_C);

		if(nactive==active && FD::time>4) setActive(1);
	}
};

void gameStart()
{
	WindowProps::setLockType(WindowProps::BARS);
	SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

	loadResources();

	//Scene::states = {new S_Splash(), new S_A(), new S_B()};
	Scene::scenes = {new S_A()};
	Scene::allStart();
	Scene::getActive()->load();
}

void gameLoop()
{
	Scene::getActive()->loop();
	Scene::update();
	common::renderFPS({});
}
void gameKeyDown(const SDL_Keycode& key)
{
	switch(key)
	{
		case SDLK_q :
			std::exit(0);
			break;
	}
	Scene::getActive()->onKey(key);
}
