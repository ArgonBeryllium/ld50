#pragma once
#include <SDL2/SDL_rect.h>
#include <vector>
#include <algorithm>
#include <cumt/cumt_aabb.h>
#include <cumt/cumt_things.h>
#include <shitrndr.h>
#include "resources.h"
#include "scenes.h"
using namespace cumt;

struct FloorTile : Thing2D
{
	static std::vector<FloorTile*> tiles;
	FloorTile(v2f pos_, v2f scl_ = {2,2}) : Thing2D(pos_, scl_)
	{
		tiles.push_back(this);
	}
	~FloorTile()
	{
		auto it = std::find(tiles.begin(), tiles.end(), this);
		tiles.erase(it);
	}
	void render() override
	{
		using namespace shitrndr;
		SetColour({55,55,55,255});
		FillRect(getRect());
	}
};
struct RoundTile : FloorTile
{
	float r, rs;
	RoundTile(v2f pos_, float r_ = 1) : r(r_), rs(r_*r_), FloorTile(pos_-v2f{r_,r_}, {r_*2,r_*2}) {}
	void render() override;
};

inline bool getPreciseOverlap(const Thing2D* a, v2f p)
{
	return p.x > a->pos.x && p.x < a->pos.x+a->scl.x &&
			p.y > a->pos.y && p.y < a->pos.y+a->scl.y;
}
inline bool getPreciseOverlap(const RoundTile* a, v2f p)
{
	return (a->centre()-p).getLengthSquare()<a->rs;
}

struct FloorMember
{
	Thing2D* parent;
	v2f pp;

	FloorMember(Thing2D* parent_) : parent(parent_) {}

	void update()
	{
		v2f d = pp-parent->pos;
		
		for(auto t : FloorTile::tiles)
		{
			RoundTile* r = dynamic_cast<RoundTile*>(t);
			if(r) { if(!getPreciseOverlap(r, parent->centre())) continue; }
			else if(!getPreciseOverlap(t, parent->centre())) continue;
			pp = parent->pos;
			return;
		}
		parent->pos += d;
	}
};

struct Goal : Thing2D
{
	static Goal* instance;
	Goal(v2f pos_) : Thing2D(pos_-v2f{2,2}, {4,4}) { instance = this; }
	~Goal() { instance = nullptr; }
	void update() override;
	void render() override
	{
		v2i sc = spaceToScr(centre());
		s_qdcircle(sc, scl.x*getScalar()/FONT_SIZE, ".!"[int(FD::time)%2], C_GOAL);
	}
};
