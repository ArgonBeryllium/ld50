#pragma once
#include <SDL2/SDL_keycode.h>
#include <cstdint>
#include <cumt/cumt.h>
#include <cumt/cumt_common.h>
#include <cumt/cumt_render.h>
#include <cumt/cumt_things.h>
#include <shitrndr.h>
#include "expirables.h"
#include "fsm.h"
#include "fsm.h"
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
	EMoveState(LaCreatura* parent_) : CreatureState(parent_) {}
	void enter() override;
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
	CreatureFSM fsm;
	State *idle_state, *move_state, *roll_state, *attack_state = new AttackState(this), *hurt_state = new HurtState(this);
	TransitionState* transition_state = new TransitionState(this);
	float hp, max_hp,
		  speed,
		  stamina, max_stamina,
		  attack_cooldown, hurt_cooldown,
		  strength, range;
	v2f target;

	LaCreatura(v2f pos_, v2f scl_, float hp_, float speed_ = 4) :
		Thing2D(pos_, scl_), hp(hp_), max_hp(hp_), speed(speed_) {} // remember to start the fsm

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
		fsm.update();
	}
	void render() override
	{
		fsm.render();
		using namespace shitrndr;

		SDL_Rect sr = getRect();
		sr.y -= 5;
		sr.h *= .2;
		SetColour({15,15,15,255});
		FillRect(sr);
		SetColour({15,150,15,255});
		sr.w *= stamina/max_stamina;
		FillRect(sr);

		SDL_Rect hr = getRect();
		hr.y -= 8;
		hr.h *= .2;
		SetColour({15,15,15,255});
		FillRect(hr);
		hr.w *= hp/max_hp;
		SetColour({150,15,15,255});
		FillRect(hr);
	}

	void onKey(SDL_Keycode key)     { fsm.onKey(key); }
	void onKeyUp(SDL_Keycode key)   { fsm.onKeyUp(key); }
	void onKeyHeld(SDL_Keycode key) { fsm.onKeyHeld(key); }
	void onMB(uint8_t b) { fsm.onMB(b); }
	void onMBUp(uint8_t b) { fsm.onMBUp(b); }
};

struct Player : LaCreatura
{
	static Player* instance;
	Player(v2f pos_ = {}) : LaCreatura(pos_, {1,1}, 2)
	{
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
		instance = this;
	}
	void update() override
	{
		target = scrToSpace(shitrndr::Input::getMP());
		LaCreatura::update();
	}
	void render() override
	{
		LaCreatura::render();
		render::text(Thing2D::spaceToScr(centre()), "P");
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
		hurt_cooldown = 1;
		strength = .6;
		range = 1.5;

		fsm.starting_state = idle_state;
		fsm.start();

		target = pos+v2f{common::frand(), common::frand()}*3;
	}
	void update() override
	{
		target = Player::instance->centre();
		LaCreatura::update();
	}
};
