#pragma once
#include "resources.h"
#include <SDL2/SDL_render.h>
#include <cstdint>
#include <cumt/cumt_things.h>
#include <cumt/cumt_render.h>
#include <shitrndr.h>
#include <vector>

struct Scene
{
	static std::vector<Scene*> scenes;
	static size_t active, nactive;
	static void setActive(size_t i);
	static Scene* getActive();
	size_t index;

	static void allStart();
	static void update();

	float ti_dur = .4, to_dur = .2;

	cumt::ThingSet set;

	virtual void transOut(float t)
	{
		using namespace shitrndr;
		using namespace cumt;
		SetColour(bg_col);
		int y = int(WindowProps::getHeight()*t);
		FillRect({0,0,WindowProps::getWidth(),y});
	}
	virtual void transIn(float t)
	{
		transOut(1-t);
	}

	virtual void start() {}
	virtual void load() {}
	virtual void unload() {}

	virtual void loop()
	{
		set.update();
		set.render();
	}
	virtual void onKey(SDL_Keycode key) {}
	virtual void onKeyUp(SDL_Keycode key) {}
	virtual void onKeyHeld(SDL_Keycode key) {}
	virtual void onMB(uint8_t b) {}
	virtual void onMBUp(uint8_t b) {}
};

struct StatusScene : Scene
{
	static StatusScene* instance;
	static SDL_Texture* banner;
	static std::string subtitle;
	static size_t score;
	static void win()
	{
		if(nactive == instance->index) return;
		score++;
		using namespace cumt;
		Scene::setActive(instance->index);
		render::TextData td;
		if(banner) SDL_DestroyTexture(banner);
		td.destroy = false;
		td.render = false;
		td.col = {255,255,0,255};
		if(score==4)
		{
			banner = render::text({}, "u win :)", td);
			subtitle = std::to_string(score)+" / 4  [Q] QUIT";
		}
		else
		{
			banner = render::text({}, "DEPTHS CONQUERED", td);
			subtitle = std::to_string(score)+" / 4  [SPACE] ADVANCE";
		}
	}
	static void lose()
	{
		if(nactive == instance->index) return;
		score = 0;
		using namespace cumt;
		Scene::setActive(instance->index);
		render::TextData td;
		if(banner) SDL_DestroyTexture(banner);
		td.destroy = false;
		td.render = false;
		td.col = {255,0,0,255};
		banner = render::text({}, "TRIAL FAILED", td);
		subtitle = "[SPACE] REBIRTH";
	}

	StatusScene()
	{
		instance = this;
	}
	
	void loop() override
	{
		using namespace shitrndr;
		using namespace cumt;
		render::texture_with_td(banner, WindowProps::getSize()/2, {});
		render::text({WindowProps::getWidth()/2, WindowProps::getHeight()*2/3}, subtitle);
	}

	void onKey(SDL_Keycode key) override
	{
		if(score < 4 && key==SDLK_SPACE)
			setActive(0); //TODO make this less hacky
	}
};
