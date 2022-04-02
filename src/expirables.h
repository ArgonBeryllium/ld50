#pragma once
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
