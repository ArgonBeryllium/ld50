#pragma once
#include <cumt/cumt_aabb.h>
#include <cumt/cumt_things.h>
#include <vector>
using namespace cumt;

/*
static inline std::vector<std::tuple<Thing2D*, v2f, v2f>> castBox(ThingSet* s, v2f pos, v2f scl)
{
	std::vector<std::tuple<Thing2D*, v2f, v2f>> out;
	v2i sp = Thing2D::spaceToScr(pos), ss = (scl*Thing2D::getScalar()).to<int>();
	SDL_Rect test_rect = {sp.x, sp.y, ss.x, ss.y};
	for(auto a : s->things_id)
		if(auto b = dynamic_cast<Thing2D*>(a.second))
			if(SDL_Rect* r = aabb::getOverlap(b->getRect(), test_rect))
			{
				v2f p, q;
				p = Thing2D::scrToSpace({r->x, r->y});
				q = Thing2D::scrToSpace({r->x+r->w, r->y+r->h});
				out.push_back(std::make_tuple(b, p, q));
				delete r;
			}
	return out;
}
*/

static inline std::vector<Thing2D*> castBox(ThingSet* s, v2f pos, v2f scl)
{
	std::vector<Thing2D*> out;
	v2i sp = Thing2D::spaceToScr(pos), ss = (scl*Thing2D::getScalar()).to<int>();
	SDL_Rect test_rect = {sp.x, sp.y, ss.x, ss.y};
	for(auto a : s->things_id)
		if(auto b = dynamic_cast<Thing2D*>(a.second))
			if(SDL_Rect* r = aabb::getOverlap(b->getRect(), test_rect))
			{
				out.push_back(b);
				delete r;
			}
	return out;
}

