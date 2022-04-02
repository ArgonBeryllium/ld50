#include "creaturas.h"
#include "attack.h"
#include "expirables.h"
#include <cstdint>
#include <cumt/cumt_common.h>
#include <cumt/cumt_things.h>

static const float stamina_regen_rate_idle = .6, stamina_regen_rate_moving = .3;
static const float roll_cost = .3, attack_cost = .4;
static void click(const uint8_t& b, LaCreatura* parent)
{
	switch(b)
	{
		case 1:
			if(parent->stamina<roll_cost) break;
			parent->fsm.enter_state(parent->roll_state);
			break;
		case 3:
			if(parent->stamina<attack_cost) break;
			parent->transition_state->transition(.2, parent->attack_state);
			break;
	}
}

void CreatureState::damage(float d)
{
	parent->hp -= d;
	if(parent->hp<=0) parent->die();
}

void IdleState::enter() {}
void IdleState::exit() {}
void IdleState::update()
{
	if(common::inVec().getLengthSquare())
		parent->fsm.enter_state(parent->move_state);

	parent->stamina += FD::delta*stamina_regen_rate_idle;
	parent->stamina = std::min(parent->stamina, parent->max_stamina);
}
void IdleState::render()
{
	using namespace shitrndr;
	SetColour({200,200,200,255});
	auto r = parent->getRect();
	r.y += std::sin(FD::time*20)*Thing2D::getScalar()*.1;
	FillRect(r);
}
void IdleState::onMB(uint8_t b)
{
	click(b, parent);
}

static float mv_t, mv_t_max = .1;
void MoveState::enter()
{
	mv_t = mv_t_max;
}
void MoveState::update()
{
	if(!common::inVec().getLengthSquare())
		parent->fsm.enter_state(parent->idle_state);
	else if(mv_t<=0)
		parent->pos += common::inVec().normalised()*FD::delta*parent->speed;
	else
	{
		mv_t -= FD::delta;
		parent->pos += common::inVec().normalised()*FD::delta*parent->speed*(1-mv_t/mv_t_max);
	}

	parent->stamina += FD::delta*stamina_regen_rate_moving;
	parent->stamina = std::min(parent->stamina, parent->max_stamina);
}
void MoveState::render()
{
	using namespace shitrndr;
	SetColour({180,200,255,255});
	FillRect(parent->getRect());
}
void MoveState::onMB(uint8_t b)
{
	click(b, parent);
}

void TransitionState::transition(float delay, State *next_state_)
{
	t = delay;
	next_state = next_state_;
	parent->fsm.enter_state(this);
}
void TransitionState::update()
{
	t -= FD::delta;
	if(t<=0) parent->fsm.enter_state(next_state);
}
void TransitionState::render()
{
	using namespace shitrndr;
	SetColour({210,210,210,Uint8(std::abs(std::sin(FD::time*4))*255)});
	FillRect(parent->getRect());
}

static constexpr float roll_dur = .2;
void RollState::enter()
{
	parent->stamina -= roll_cost;

	v2f d = Thing2D::scrToSpace(shitrndr::Input::getMP())-parent->centre();
	tp = parent->pos+d.normalised()*2;
	ip = parent->pos;
	t = roll_dur;
}
void RollState::update()
{
	t -= FD::delta;
	parent->pos = common::lerp(ip, tp, 1-t/roll_dur);
	if(t<=0) parent->transition_state->transition(.5, parent->idle_state);
}
void RollState::render()
{
	using namespace shitrndr;
	SetColour({180,200,255,205});
	FillRect(parent->getRect());
}

void AttackState::enter()
{
	parent->stamina -= attack_cost;

	t = parent->attack_cooldown;

	next_state = parent->idle_state;
	v2f d = Thing2D::scrToSpace(shitrndr::Input::getMP())-parent->pos;
	d = d.normalised();
	parent->parent_set->instantiate(new Attack(parent, parent->pos+d));
}
void AttackState::render()
{
	using namespace shitrndr;
	SetColour({10,20,255,255});
	FillRect(parent->getRect());
}

void HurtState::enter()
{
	t = parent->attack_cooldown;
	next_state = parent->idle_state;
	v2f d = Thing2D::scrToSpace(shitrndr::Input::getMP())-parent->centre();
	d = d.normalised();
	parent->parent_set->instantiate(new Expirable(1, parent->centre()+d));
}
void HurtState::render()
{
	using namespace shitrndr;
	SetColour({255,10,10,Uint8(std::abs(std::sin(FD::time*4))*255)});
	FillRect(parent->getRect());
}
