#pragma once
#include <algorithm>
#include <cstdint>
#include <cumt/cumt.h>
#include <shitrndr.h>
#include "expirables.h"
#include "fsm.h"
#include "fsm.h"
#include "level.h"
#include "resources.h"
#include "scenes.h"
using namespace cumt;

struct TransitionState : CreatureState
{
	float t = .5;
	State* next_state;
	TransitionState(LaCreatura* parent_) : CreatureState(parent_) {}
	void transition(float delay, State* next_state_);
	void update() override;
	void render() override;
};

struct PIdleState : CreatureState
{
	PIdleState(LaCreatura* parent_) : CreatureState(parent_) {}

	void enter() override;
	void exit() override;
	void update() override;
	void render() override;
	void onMB(uint8_t b) override;
};
struct PMoveState : CreatureState
{
	PMoveState(LaCreatura* parent_) : CreatureState(parent_) {}
	void enter() override;
	void update() override;
	void render() override;
	void onMB(uint8_t b) override;
};
struct PRollState : CreatureState
{
	v2f ip, tp;
	float t;
	PRollState(LaCreatura* parent_) : CreatureState(parent_) {}
	void enter() override;
	void update() override;
	void render() override;
	void damage(float d) override {}
};

struct EIdleState : CreatureState
{
	float dur;
	EIdleState(LaCreatura* parent_) : CreatureState(parent_) {}

	void enter() override;
	void update() override;
	void render() override;
};
struct EMoveState : CreatureState
{
	float dur;
	bool chasing;
	EMoveState(LaCreatura* parent_) : CreatureState(parent_) {}
	void enter() override;
	void exit() override;
	void update() override;
	void render() override;
};
struct ERollState : CreatureState
{
	v2f ip, tp;
	float t;
	ERollState(LaCreatura* parent_) : CreatureState(parent_) {}
	void enter() override;
	void update() override;
	void render() override;
	void damage(float d) override {}
};

struct AttackState : TransitionState
{
	AttackState(LaCreatura* parent_) : TransitionState(parent_) {}
	void enter() override;
	void render() override;
};
struct HurtState : TransitionState
{
	HurtState(LaCreatura* parent_) : TransitionState(parent_) {}
	void enter() override;
	void render() override;
};

// dios mio
struct LaCreatura : Thing2D
{
	std::wstring spr_wait = SPR_SNAIL_WAIT, spr_def = SPR_SNAIL;
	CreatureFSM fsm;
	State *idle_state, *move_state, *roll_state, *attack_state = new AttackState(this), *hurt_state = new HurtState(this);
	TransitionState* transition_state = new TransitionState(this);
	float hp, max_hp,
		  speed,
		  stamina, max_stamina,
		  attack_cooldown, hurt_cooldown,
		  strength, range,
		  detection_radius, chase_radius; // these should be in the Enemy class but at this point I can't be bothered to write out the pointer casts in the states
	v2f target;
	FloorMember fm = FloorMember(this);

	static std::vector<LaCreatura*> las_creaturas;
	LaCreatura(v2f pos_, v2f scl_, float hp_, float speed_ = 4) :
		Thing2D(pos_, scl_), hp(hp_), max_hp(hp_), speed(speed_) { las_creaturas.push_back(this); } // remember to start the fsm
	~LaCreatura()
	{
		auto i = std::find(las_creaturas.begin(), las_creaturas.end(), this);
		las_creaturas.erase(i);
	}

	bool is_dead() { return hp<=0; }
	virtual void die()
	{
		Particles2D* p = new Particles2D(30);
		parent_set->instantiate(p);
		parent_set->scheduleDestroy(this);
	}
	void damage(float d)
	{
		((CreatureState*)fsm.current_state)->damage(d);
	}

	void update() override
	{
		if(!onScreen(getRect())) return;
		fsm.update();
		fm.update();
	}

	void renderBar(v2i p, float f, int col, wchar_t c)
	{
		s_qline(p, p+v2i(getScalar()*f, 0), c, col);
		put_char(quantisePos(p), '[');
		put_char(quantisePos(p+v2i(getScalar(),0)), ']');
	}
	void render() override
	{
		if(!onScreen(getRect())) return;
		fsm.render();
		using namespace shitrndr;

		v2i bp = spaceToScr(pos);
		bp.y -= 32;
		renderBar(bp, hp/max_hp, C_HURT, '#');
		bp.y+=13;
		renderBar(bp, hp/max_hp, C_GRAY, '-');
	}

	void onKey(SDL_Keycode key)     { fsm.onKey(key); }
	void onKeyUp(SDL_Keycode key)   { fsm.onKeyUp(key); }
	void onKeyHeld(SDL_Keycode key) { fsm.onKeyHeld(key); }
	void onMB(uint8_t b) { fsm.onMB(b); }
	void onMBUp(uint8_t b) { fsm.onMBUp(b); }
};

struct Player : LaCreatura
{
	float flame_fade = .05;
	static Player* instance;

	float base_flame = 1;
	inline float getFlame() { return base_flame*stamina/max_stamina; }

	Player(v2f pos_ = {}) : LaCreatura(pos_, {1,1}, 2)
	{
		spr_wait = SPR_P_WAIT;
		spr_def = SPR_P;
		idle_state = new PIdleState(this);
		move_state = new PMoveState(this);
		roll_state = new PRollState(this);

		stamina = max_stamina = 2;
		attack_cooldown = .3;
		hurt_cooldown = .3;
		strength = .6;
		range = 1.5;

		fsm.starting_state = idle_state;
		fsm.start();

		if(instance) std::cerr << "UH OH! multiple player instances\n";
		instance = this;
	}
	~Player()
	{
		instance = nullptr;
	}
	void update() override
	{
		target = scrToSpace(shitrndr::Input::getMP());

		LaCreatura::update();
		base_flame -= flame_fade*FD::delta;
		if(getFlame()<=0)
			die();
	}
	void render() override
	{
		static v2f fpt[3] = {{},{},{}};
		v2f fp = Thing2D::scrToSpace(shitrndr::Input::getMP())-centre();
		fp = fp.normalised();
		fp += centre();
		char c = FLAME_CHARS[std::min(9, int(getFlame()*2)+int(FD::time*6)%2)];
		int coli = std::min(4, int(getFlame()));

		s_qfcircle(spaceToScr(fp), getFlame(), L'░', 0, CS_FLAME[coli]);
		if(coli>2) put_char(quantisePos(Thing2D::spaceToScr(fpt[0])), c, C_FG, FLAME_CHARS[coli-2]);
		if(coli>1) put_char(quantisePos(Thing2D::spaceToScr(fpt[1])), c, C_FG, FLAME_CHARS[coli-1]);
		if(coli>0) put_char(quantisePos(Thing2D::spaceToScr(fpt[2])), c, C_FG, FLAME_CHARS[coli-1]);
		put_char(quantisePos(Thing2D::spaceToScr(fp)), c, C_FG, CS_FLAME[coli]);
		fpt[0] = fpt[1];
		fpt[1] = fpt[2];
		fpt[2] = fp;

		fsm.render();
		using namespace shitrndr;

		v2i bp = spaceToScr(pos);
		bp.y -= 32;
		renderBar(bp, hp/max_hp, C_HURT, '#');
	}

	virtual void die() override
	{
		StatusScene::lose();
	}
};

struct Enemy : LaCreatura
{
	Enemy(v2f pos_, v2f scl_ = {1,1}, float hp_ = 2, float speed_ = 3) :
		LaCreatura(pos_, scl_, hp_, speed_)
	{
		idle_state = new EIdleState(this);
		move_state = new EMoveState(this);
		roll_state = new ERollState(this);

		stamina = max_stamina = 1;
		attack_cooldown = .6;
		hurt_cooldown = .4;
		strength = .6;
		range = 1.5;
		detection_radius = 5;
		chase_radius = 9;

		fsm.starting_state = idle_state;
		fsm.start();

		target = pos+v2f{common::frand(), common::frand()}*3;
	}
	void update() override
	{
		LaCreatura::update();
	}

	virtual void die() override
	{
		Player::instance->base_flame += max_hp;
		parent_set->scheduleDestroy(this);
		parent_set->instantiate(new Particles(centre(), C_HURT, ',', 1));
	}
};
