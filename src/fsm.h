#pragma once

#include <SDL2/SDL_keycode.h>
#include <iostream>
struct State
{
	virtual void enter() {}
	virtual void exit() {}
	virtual void update() {}
	virtual void render() {}
};
struct FiniteStateMachine
{
	State* current_state = nullptr;
	State* starting_state;

	void enter_state(State* state)
	{
		if(!state)
		{
			std::cout << "attempting to switch to nonexistent state\n";
			return;
		}
		if(current_state)
			current_state->exit();
		current_state = state;
		state->enter();
	}

	void start()
	{
		if(starting_state)
			enter_state(starting_state);
	}
	void update()
	{
		if(current_state)
			current_state->update();
	}
	void render()
	{
		if(current_state)
			current_state->render();
	}
};

struct LaCreatura;
struct CreatureState : State
{
	LaCreatura* parent;
	CreatureState(LaCreatura* parent_) : parent(parent_) {}
	virtual void onKey(SDL_Keycode key) {}
	virtual void onKeyUp(SDL_Keycode key) {}
	virtual void onKeyHeld(SDL_Keycode key) {}
	virtual void onMB(uint8_t b) {}
	virtual void onMBUp(uint8_t b) {}
	virtual void damage(float d);
};
struct CreatureFSM : FiniteStateMachine
{
	virtual void onKey(SDL_Keycode key)
	{
		if(CreatureState* c = dynamic_cast<CreatureState*>(current_state))
			c->onKey(key);
	}
	virtual void onKeyUp(SDL_Keycode key)
	{
		if(CreatureState* c = dynamic_cast<CreatureState*>(current_state))
			c->onKeyUp(key);
	}
	virtual void onKeyHeld(SDL_Keycode key)
	{
		if(CreatureState* c = dynamic_cast<CreatureState*>(current_state))
			c->onKeyHeld(key);
	}
	virtual void onMB(uint8_t b)
	{
		if(CreatureState* c = dynamic_cast<CreatureState*>(current_state))
			c->onMB(b);
	}
	virtual void onMBUp(uint8_t b)
	{
		if(CreatureState* c = dynamic_cast<CreatureState*>(current_state))
			c->onMBUp(b);
	}
	virtual void damage(float d)
	{
		if(CreatureState* c = dynamic_cast<CreatureState*>(current_state))
			c->damage(d);
	}
};
