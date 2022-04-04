#include "creaturas.h"
#include "attack.h"
#include "expirables.h"
#include "resources.h"
#include <cstdint>
#include <cumt/cumt_common.h>
#include <cumt/cumt_things.h>
#include <shitrndr.h>

Player* Player::instance = nullptr;
std::vector<LaCreatura*> LaCreatura::las_creaturas;
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

static void render_lc(LaCreatura* p, std::wstring s = L"", const uint32_t& c = C_FG)
{
	if(s.empty()) s = p->spr_def;
	v2i ss = (p->scl*Thing2D::getScalar()).to<int>();
	v2i sp = Thing2D::spaceToScr(p->pos);
	bool a = (p->target-p->centre()).x<0;
	v2i fo = sp-v2i(a?-ss.x:0, ss.y*.3);

	if(p==Player::instance)
		put_qstring(fo, s, a, c, CS_LEVEL[StatusScene::instance->score]);
	else if(((Player::instance->centre()-p->centre()).getLengthSquare()/16 < std::max(1.f, Player::instance->getFlame()))*4)
		put_qstring(fo, s, a, c, CS_LEVEL[StatusScene::instance->score]);
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
	render_lc(parent, parent->spr_wait);
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
	render_lc(parent);
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
	render_lc(parent, SPRS_P_WALK[int(FD::time*3)%2]);
}
void PMoveState::onMB(uint8_t b)
{
	click(b, parent);
}

static constexpr float roll_dur = .2, roll_vuln = .3;
void PRollState::enter()
{
	if(parent->stamina<roll_cost)
	{
		parent->fsm.enter_state(parent->idle_state);
		return;
	}
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
	if(t<=0) parent->transition_state->transition(roll_vuln, parent->idle_state);
}
void PRollState::render()
{
	render_lc(parent, SPRS_P_ROLL[int(FD::time*2)%2]);
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
	else
	{
		parent->target = parent->centre()+v2f(common::frand()*2-1,common::frand()*2-1)*parent->speed*5;
		parent->fsm.enter_state(parent->move_state);
	}

	v2f cs = v2f(1,1)*3;
	if((Player::instance->centre()-parent->centre()).getLengthSquare()<parent->detection_radius*parent->detection_radius)
	{
		parent->target = Player::instance->centre();
		parent->fsm.enter_state(parent->move_state);
	}

	parent->stamina += FD::delta*stamina_regen_rate_idle;
	parent->stamina = std::min(parent->stamina, parent->max_stamina);
}
void EIdleState::render()
{
	render_lc(parent);
}

void EMoveState::enter()
{
	dur = common::frand()*2+3;
}
void EMoveState::exit()
{
	chasing = false;
}
void EMoveState::update()
{
	dur -= FD::delta;
	v2f d = parent->target-parent->pos;
	parent->pos += d.normalised()*FD::delta*parent->speed;

	float pds = (Player::instance->centre()-parent->centre()).getLengthSquare();
	float drs = parent->detection_radius;
	drs = drs*drs;
	float crs = parent->chase_radius;
	crs = crs*crs;

	if(pds < drs) chasing = true;
	if((!chasing && pds>drs && dur<=0) || (chasing && pds>crs))
		parent->fsm.enter_state(parent->idle_state);
	else if(chasing && pds<crs)
		parent->target = Player::instance->centre();
	if(pds<parent->range+parent->scl.x)
		parent->fsm.enter_state(common::frand() < parent->hp/parent->max_hp*.1+.8 ? parent->attack_state:parent->roll_state);

	parent->stamina += FD::delta*stamina_regen_rate_moving;
	parent->stamina = std::min(parent->stamina, parent->max_stamina);
}
void EMoveState::render()
{
	render_lc(parent);
}

void ERollState::enter()
{
	if(parent->stamina<roll_cost)
	{
		parent->fsm.enter_state(parent->idle_state);
		return;
	}
	parent->stamina -= roll_cost;

	v2f d = parent->centre()-Player::instance->centre();
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
	render_lc(parent);
}
// ########### //

// ## general ## //
void AttackState::enter()
{
	if(parent==Player::instance)
	{
		parent->parent_set->instantiate(new Particles(parent->centre(), C_HURT, '-',1, 5));
	}
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
	render_lc(parent);
}

void HurtState::enter()
{
	t = parent->hurt_cooldown;
	next_state = parent->idle_state;
	parent->parent_set->instantiate(new Particles(parent->centre(), C_HURT, '*',1, 5));
}
void HurtState::render()
{
	render_lc(parent, parent->spr_wait, C_HURT);
}
