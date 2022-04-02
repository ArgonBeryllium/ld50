#pragma once
#include "collisions.h"
#include "expirables.h"
#include "creaturas.h"
#include "fsm.h"
using namespace cumt;

struct Attack : Expirable
{
	LaCreatura* parent;
	float dmg;
	Attack(LaCreatura* parent_, v2f pos_, float dmg_ = .5, float scl_ = 1) : Expirable(.1, pos_, {scl_, scl_}), parent(parent_), dmg(dmg_) {}

	void update() override
	{
		auto cols = castBox(parent_set, pos, scl);
		for(auto c : cols)
			if(auto l = dynamic_cast<LaCreatura*>(c))
			{
				if(l==parent) continue;
				l->damage(dmg);
				parent_set->scheduleDestroy(this);
				return;
			}
		Expirable::update();
	}
};
