#ifndef TELEOP_H
#define TELEOP_H

#include "executive.h"
#include "../util/posedge_trigger_debounce.h"
#include "../util/posedge_toggle.h"
#include "../control/tilt_presets.h"
#include "../control/shooter_constants.h"

struct Teleop : Executive_impl<Teleop> {
	Shooter_constants shooter_constants;
	Tilt_presets tilt_presets;
	
	#define SHOOT_STEPS X(SPEED_UP) X(SHOOT)
	enum class Shoot_steps{
		#define X(NAME) NAME,
		SHOOT_STEPS
		#undef X
	};	
	Shoot_steps shoot_step;
	
	Countdown_timer shoot_high_timer, shoot_low_timer, cheval_lift_timer, cheval_drive_timer;
	
	#define COLLECTOR_MODES X(CHEVAL) X(NOTHING) X(COLLECT) X(STOW) X(SHOOT_HIGH) X(REFLECT) X(SHOOT_LOW) X(LOW) X(DRAWBRIDGE)
	enum class Collector_mode{
		#define X(name) name,
		COLLECTOR_MODES
		#undef X
	};
	Collector_mode collector_mode;
	
	enum Nudges{FORWARD,BACKWARD,CLOCKWISE,COUNTERCLOCKWISE,NUDGES};
	#define NUDGE_ITEMS(X) X(Posedge_trigger,trigger) X(Countdown_timer,timer)
	struct Nudge{
		Posedge_trigger trigger;
		Countdown_timer timer;
	};
	std::array<Nudge,NUDGES> nudges;
	
	Posedge_toggle controller_auto;
	Posedge_trigger_debounce cheval_trigger;
	bool tilt_learn_mode;

	#define CHEVAL_STEPS X(GO_DOWN) X(DRIVE) X(DRIVE_AND_STOW)
	enum class Cheval_steps{
		#define X(NAME) NAME,
		CHEVAL_STEPS
		#undef X
	};
	Cheval_steps cheval_step;

	#define JOY_COLLECTOR_POS X(STOP) X(LOW) X(LEVEL)
	enum class Joy_collector_pos{
		#define X(name) name,
		JOY_COLLECTOR_POS
		#undef X
	};
	Joy_collector_pos joy_collector_pos;	

	Posedge_trigger_debounce set_button;
	
	Shooter::Goal shoot_action(Panel::Shooter_mode,double,bool)const;
	void shooter_protocol(Toplevel::Status_detail const&,const bool,const Time,Toplevel::Goal&,bool,Panel::Shooter_mode,double);
	void cal(Time,double,double,Panel const&);

	Executive next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator<(Teleop const&)const;
	bool operator==(Teleop const&)const;
	void display(std::ostream&)const;
	Teleop();
};

std::ostream& operator<<(std::ostream&,Teleop::Shoot_steps);
std::ostream& operator<<(std::ostream&,Teleop::Collector_mode);
std::ostream& operator<<(std::ostream&,Teleop::Cheval_steps);
std::ostream& operator<<(std::ostream&,Teleop::Joy_collector_pos);
std::ostream& operator<<(std::ostream&,Teleop::Nudge const&);

bool operator<(Teleop::Nudge const&,Teleop::Nudge const&);
bool operator==(Teleop::Nudge const&,Teleop::Nudge const&);
double set_drive_speed(double,double,double);

#endif