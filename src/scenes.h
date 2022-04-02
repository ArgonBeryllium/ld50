#pragma once
#include <cstdint>
#include <cumt/cumt_things.h>
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

	float ti_dur = .2, to_dur = .2;

	cumt::ThingSet set;

	virtual void transOut(float t)
	{
		using namespace shitrndr;
		SetColour(bg_col);
		FillRect({0,0,WindowProps::getWidth(), int(WindowProps::getHeight()*t)});
	}
	virtual void transIn(float t)
	{
		using namespace shitrndr;
		SetColour(bg_col);
		FillRect({0,0,WindowProps::getWidth(), int(WindowProps::getHeight()*(1-t))});
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
