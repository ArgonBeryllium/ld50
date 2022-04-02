#pragma once
#include <SDL2/SDL_keycode.h>
#include <cstdint>
#include <cumt/cumt.h>
#include <cumt/cumt_things.h>
#include <shitrndr.h>
#include "expirables.h"
#include "fsm.h"
#include "fsm.h"
using namespace cumt;

struct IdleState : CreatureState
{
	IdleState(LaCreatura* parent_) : CreatureState(parent_) {}

	void enter() override;
	void exit() override;
	void update() override;
	void render() override;
	void onMB(uint8_t b) override;
};
struct MoveState : CreatureState
{
	MoveState(LaCreatura* parent_) : CreatureState(parent_) {}
	void enter() override;
	void update() override;
	void render() override;
	void onMB(uint8_t b) override;
};
struct TransitionState : CreatureState
{
	float t = .5;
	State* next_state;
	TransitionState(LaCreatura* parent_) : CreatureState(parent_) {}
	void transition(float delay, State* next_state_);
	void update() override;
	void render() override;
};
struct RollState : CreatureState
{
	v2f ip, tp;
	float t;
	RollState(LaCreatura* parent_) : CreatureState(parent_) {}
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
	State* idle_state = new IdleState(this),
			*move_state = new MoveState(this),
			*roll_state = new RollState(this),
			*attack_state = new AttackState(this),
			*hurt_state = new HurtState(this);
	TransitionState* transition_state = new TransitionState(this);
	float hp, max_hp,
		  speed,
		  stamina, max_stamina,
		  attack_cooldown;

	LaCreatura(v2f pos_, v2f scl_, float hp_, float speed_ = 4) :
		Thing2D(pos_, scl_), hp(hp_), max_hp(hp_), speed(speed_)
	{
		fsm.starting_state = idle_state;
		fsm.start();

		stamina = max_stamina = 1;
		attack_cooldown = .3;
	}

	bool is_dead() { return hp<=0; }
	virtual void die()
	{
		Particles2D* p = new Particles2D(30);
		parent_set->instantiate(p);
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

	virtual void onKey(SDL_Keycode key)     { fsm.onKey(key); }
	virtual void onKeyUp(SDL_Keycode key)   { fsm.onKeyUp(key); }
	virtual void onKeyHeld(SDL_Keycode key) { fsm.onKeyHeld(key); }
	virtual void onMB(uint8_t b) { fsm.onMB(b); }
	virtual void onMBUp(uint8_t b) { fsm.onMBUp(b); }
};
