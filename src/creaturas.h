#pragma once
#include <SDL2/SDL_keycode.h>
#include <cstdint>
#include <cumt/cumt.h>
#include <cumt/cumt_things.h>
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

// dios mio
struct LaCreatura : Thing2D
{
	CreatureFSM fsm;
	State* idle_state = new IdleState(this),
			*move_state = new MoveState(this),
			*roll_state = new RollState(this),
			*hurt_state;
	TransitionState* transition_state = new TransitionState(this);
	float hp, speed;

	LaCreatura(v2f pos_, v2f scl_, float hp_, float speed_ = 4) : Thing2D(pos_, scl_), hp(hp_), speed(speed_)
	{
		fsm.starting_state = idle_state;
		fsm.start();
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
	}

	virtual void onKey(SDL_Keycode key)     { fsm.onKey(key); }
	virtual void onKeyUp(SDL_Keycode key)   { fsm.onKeyUp(key); }
	virtual void onKeyHeld(SDL_Keycode key) { fsm.onKeyHeld(key); }
	virtual void onMB(uint8_t b) { fsm.onMB(b); }
	virtual void onMBUp(uint8_t b) { fsm.onMBUp(b); }
};
