#include "creaturas.h"
#include "attack.h"
#include "expirables.h"
#include <cstdint>
#include <cumt/cumt_common.h>
#include <cumt/cumt_things.h>
#include <shitrndr.h>

Player* Player::instance = nullptr;
static const float stamina_regen_rate_idle = .6, stamina_regen_rate_moving = .3;
static const float roll_cost = .3, attack_cost = .4;
static void click(const uint8_t& b, LaCreatura* parent)
{
	switch(b)
	{
		case 1:
			parent->fsm.enter_state(parent->attack_state);
			break;
		case 3:
			parent->fsm.enter_state(parent->roll_state);
			break;
	}
}

static void render_lc(LaCreatura* p, SDL_Colour c)
{
	using namespace shitrndr;
	SDL_Rect r = p->getRect();
	float s = 1-std::abs(WindowProps::getHeight()/2.-r.y)/WindowProps::getHeight()*2;
	SetColour(c);
	FillRect(r);
}

void CreatureState::damage(float d)
{
	parent->hp -= d;
	parent->fsm.enter_state(parent->hurt_state);
	if(parent->hp<=0) parent->die();
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
	render_lc(parent, {210,210,210,Uint8(std::abs(std::sin(FD::time*4))*255)});
}

// ## player ## //
void PIdleState::enter() {}
void PIdleState::exit() {}
void PIdleState::update()
{
	if(common::inVec().getLengthSquare())
		parent->fsm.enter_state(parent->move_state);

	parent->stamina += FD::delta*stamina_regen_rate_idle;
	parent->stamina = std::min(parent->stamina, parent->max_stamina);
}
void PIdleState::render()
{
	render_lc(parent, {200,200,200,255});
}
void PIdleState::onMB(uint8_t b)
{
	click(b, parent);
}

static float mv_t, mv_t_max = .1;
void PMoveState::enter()
{
	mv_t = mv_t_max;
}
void PMoveState::update()
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
void PMoveState::render()
{
	render_lc(parent, {180,200,255,255});
}
void PMoveState::onMB(uint8_t b)
{
	click(b, parent);
}

static constexpr float roll_dur = .2;
void PRollState::enter()
{
	parent->stamina -= roll_cost;

	v2f d = Thing2D::scrToSpace(shitrndr::Input::getMP())-parent->centre();
	tp = parent->pos+d.normalised()*2;
	ip = parent->pos;
	t = roll_dur;
}
void PRollState::update()
{
	t -= FD::delta;
	parent->pos = common::lerp(ip, tp, 1-t/roll_dur);
	if(t<=0) parent->transition_state->transition(.5, parent->idle_state);
}
void PRollState::render()
{
	render_lc(parent, {180,200,255,205});
}
// ############ //

// ## enemy ## //
void EIdleState::enter()
{
	dur = common::frand()*2+1;
}
void EIdleState::update()
{
	if(dur>0) dur -= FD::delta;
	else parent->fsm.enter_state(parent->move_state);

	v2f cs = v2f(1,1)*3;
	auto cols = castBox(parent->parent_set, parent->centre()-cs/2, cs);
	for(auto c : cols)
	{
		if(c==parent) continue;
		if(c==Player::instance)
		{
			parent->target = Player::instance->centre();
			parent->fsm.enter_state(parent->move_state);
			break;
		}
	}

	parent->stamina += FD::delta*stamina_regen_rate_idle;
	parent->stamina = std::min(parent->stamina, parent->max_stamina);
}
void EIdleState::render()
{
	render_lc(parent, {200,180,180,255});
}

void EMoveState::enter()
{
	dur = 4;
}
void EMoveState::update()
{
	dur -= FD::delta;
	v2f d = parent->target-parent->pos;
	parent->pos += d.normalised()*FD::delta*parent->speed;

	float pds = (Player::instance->pos-parent->pos).getLengthSquare();
	if(pds>100 && dur <= 0)
		parent->fsm.enter_state(parent->idle_state);
	else if(pds<parent->range+parent->scl.x)
		parent->fsm.enter_state(parent->attack_state);

	parent->stamina += FD::delta*stamina_regen_rate_moving;
	parent->stamina = std::min(parent->stamina, parent->max_stamina);
}
void EMoveState::render()
{
	render_lc(parent, {180,200,255,255});
}

void ERollState::enter()
{
	if(parent->stamina<roll_cost)
	{
		parent->fsm.enter_state(parent->idle_state);
		return;
	}
	parent->stamina -= roll_cost;

	v2f d = parent->pos-Player::instance->pos;
	tp = parent->pos+d.normalised()*2;
	ip = parent->pos;
	t = roll_dur;
}
void ERollState::update()
{
	t -= FD::delta;
	parent->pos = common::lerp(ip, tp, 1-t/roll_dur);
	if(t<=0) parent->transition_state->transition(.5, parent->idle_state);
}
void ERollState::render()
{
	render_lc(parent, {180,200,255,205});
}
// ########### //

// ## general ## //
void AttackState::enter()
{
	if(parent->stamina<attack_cost)
	{
		parent->fsm.enter_state(parent->idle_state);
		return;
	}
	parent->stamina -= attack_cost;

	t = parent->attack_cooldown;

	next_state = parent->idle_state;
	v2f d = parent->target-parent->centre();
	d = d.normalised();
	parent->parent_set->instantiate(new Attack(parent, parent->centre()+d, parent->strength, parent->range));
}
void AttackState::render()
{
	render_lc(parent, {10,20,255,255});
}

void HurtState::enter()
{
	t = parent->hurt_cooldown;
	next_state = parent->idle_state;
}
void HurtState::render()
{
	render_lc(parent, {255,10,10,Uint8(std::abs(std::sin(FD::time*4))*255)});
}
