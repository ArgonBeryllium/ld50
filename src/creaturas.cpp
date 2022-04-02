#include "creaturas.h"
#include <cumt/cumt_common.h>
#include <cumt/cumt_things.h>

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
	parent->fsm.enter_state(parent->roll_state);
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
}
void MoveState::render()
{
	using namespace shitrndr;
	SetColour({180,200,255,255});
	FillRect(parent->getRect());
}
void MoveState::onMB(uint8_t b)
{
	parent->fsm.enter_state(parent->roll_state);
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

static constexpr float mt = .2;
void RollState::enter()
{
	v2f d = Thing2D::scrToSpace(shitrndr::Input::getMP())-parent->centre();
	tp = parent->pos+d.normalised()*2;
	ip = parent->pos;
	t = mt;
}
void RollState::update()
{
	t -= FD::delta;
	parent->pos = common::lerp(ip, tp, 1-t/mt);
	if(t<=0) parent->transition_state->transition(.5, parent->idle_state);
}
void RollState::render()
{
	using namespace shitrndr;
	SetColour({180,200,255,205});
	FillRect(parent->getRect());
}
