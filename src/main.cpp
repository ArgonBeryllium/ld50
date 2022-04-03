#include <cstdint>
#include <cumt/cumt.h>
#include <cumt/cumt_things.h>
#include <shitrndr.h>
#include "creaturas.h"
#include "fsm.h"
#include "level.h"
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

void spawnTrinket(v2f p)
{
	//TODO
}
inline v2f getRandomPointAway(v2f o, float r)
{
	float a = common::frand()*M_PIf32*2;
	return o+v2f(std::cos(a)*r, std::sin(a)*r);
}
void generateLevelPath(ThingSet* set, v2f sp, v2f t, int depth = 0)
{
	int e = depth?std::rand()%9+2:20;
	while((sp-t).getLengthSquare()>5)
	{
		e--;
		if(!e)
		{
			e = std::rand()%9+1;
			set->instantiate(new Enemy(sp));
		}
		v2f d = t-sp;

		float r = (common::frand()*2+1);

		if(std::rand()%2) set->instantiate(new RoundTile(sp, r));
		else set->instantiate(new FloorTile(sp, v2f(1,1)*r*2));
		
		if(depth<3 && common::frand()<.05)
		{
			v2f p = getRandomPointAway(sp, common::frand()*20+10);
			generateLevelPath(set, sp, p, depth+1);
			spawnTrinket(p);
		}

		sp += d.normalised()*r/2+v2f(common::frand(), common::frand())*.1;
	}
}

struct S_A : Scene
{
	void load() override
	{
		bg_col = {0,0,0,255};
		set.clear();
		set.instantiate(new FloorTile({-5,-5}, {10, 10}));
		auto t = set.instantiate(new FloorTile(getRandomPointAway({}, 40+common::frand()*20), {10, 10}));
		generateLevelPath(&set, {}, t->centre());
		set.instantiate(new Player());
		//set.instantiate(new Enemy({-5, 2}));
		Thing2D::view_scale = .3;
	}
	void loop() override
	{
		render::pattern::checkerBoard(-Thing2D::view_pos.x*Thing2D::getScalar(), -Thing2D::view_pos.y*Thing2D::getScalar(), 15);
		for(auto c : LaCreatura::las_creaturas)
			for(auto d : LaCreatura::las_creaturas)
				aabb::resolveOverlaps(c, d, c->max_hp/(c->max_hp+d->max_hp));
		Scene::loop();
		for(auto c : LaCreatura::las_creaturas)
			c->render();
		Thing2D::view_pos = common::lerp(Thing2D::view_pos, Player::instance->centre(), FD::delta*4);
	}
	void onKey(SDL_Keycode key) override
	{
		for(auto p : set.things_id)
			if(LaCreatura* c = dynamic_cast<LaCreatura*>(p.second))
				c->onKey(key);
		switch(key)
		{
			case SDLK_SPACE:
				set.clear();
				load();
				break;
			case SDLK_z:
				Thing2D::view_scale = .05;
				break;
			case SDLK_x:
				Thing2D::view_scale = .3;
				break;
		}
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
