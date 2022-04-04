#pragma once
#include "resources.h"
#include <cumt/cumt_things.h>

struct Expirable : cumt::Thing2D
{
	float life, max_life;
	Expirable(float life_ = 1, cumt::v2f pos_ = {}, cumt::v2f scl_ = {1,1}) : Thing2D(pos_, scl_), life(life_), max_life(life_) {}

	virtual void onDeath() {}
	void update() override
	{
		life -= cumt::FD::delta;
		if(life<0)
		{
			parent_set->scheduleDestroy(this);
			onDeath();
		}
	}
};

using namespace cumt::common;
using namespace cumt;
struct Particles : Expirable
{
	int count;
	uint32_t col;
	wchar_t ch;
	std::vector<cumt::v2f> vels, poss;

	Particles(cumt::v2f pos_, uint32_t col_ = C_GOAL, wchar_t c_ = '*', float force = 1, int c = 20) :
		Expirable(1, pos_), ch(c_), col(col_), count(c)
	{
		for(auto i = 0; i < c; i++)
		{
			vels.push_back(v2f(.5+frand(), .5+frand())*2*force);
			poss.push_back(pos);
		}
	}

	void update() override
	{
		for(auto i = 0; i < count; i++)
		{
			vels[i] += v2f{0, static_cast<float>(FD::delta)};
			poss[i] += vels[i]*FD::delta;
		}
		Expirable::update();
	}
	void render() override
	{
		for(auto i = 0; i < count; i++)
			put_char(quantisePos(spaceToScr(poss[i])), ch, col);
	}
};
