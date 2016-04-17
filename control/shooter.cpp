#include "shooter.h"
#include <cmath>
#include <cassert>

#define SHOOTER_WHEEL_LOC 0
#define BEAM_SENSOR_DIO 5
const double GROUND_RPM=-4000.0;
const double CLIMB_RPM=-5800.0;
const double FREE_SPIN_RPM=-1000.0;

Shooter::Status_detail::Status_detail():speed(0),beam(0){}
Shooter::Status_detail::Status_detail(double s,bool b):speed(s),beam(b){}

Shooter::Estimator::Estimator():last({}),speed_up_timer({}),last_output({0,0,Talon_srx_output::Mode::VOLTAGE}){}

Shooter::Goal::Goal():mode(Shooter::Goal::Mode::SPEED_AUTO),type(Shooter::Goal::Type::STOP),percentage(1.0){}
Shooter::Goal::Goal(Shooter::Goal::Type t):mode(Shooter::Goal::Mode::SPEED_AUTO),type(t),percentage(1.0){}
Shooter::Goal::Goal(Shooter::Goal::Mode m,Shooter::Goal::Type t,float):mode(m),type(t),percentage(1.0){}

Shooter::Output::Output():speed(0),mode(Talon_srx_output::Mode::VOLTAGE){}
Shooter::Output::Output(double s,double v,Talon_srx_output::Mode m):speed(s),voltage(v),mode(m){}

std::ostream& operator<<(std::ostream& o,Shooter::Goal::Type a){
	#define X(name) if(a==Shooter::Goal::Type::name) return o<<#name")";
	SHOOTER_TYPES
	#undef X
	assert(0);
}


std::ostream& operator<<(std::ostream& o,Shooter::Goal::Mode a){
	#define X(name) if(a==Shooter::Goal::Mode::name) return o<<#name")";
	SHOOTER_MODES
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Shooter::Goal goal){
	o<<"Shooter::Goal(";
	o<<" mode:"<<goal.mode;
	o<<" type:"<<goal.type;
	o<<" percentage:"<<goal.percentage;
	return o;
}

std::ostream& operator<<(std::ostream& o,Shooter::Estimator a){ return o<<"Shooter::Estimator( last:"<<a.get()<<" last_output:"<<a.last_output<<" speed_up_timer:"<<a.speed_up_timer<<")"; }
std::ostream& operator<<(std::ostream& o,Shooter::Input a){ return o<<"Shooter::Input( speed:"<<a.speed<<" beam:"<<a.beam<<" enabled:"<<a.enabled<<")"; }
std::ostream& operator<<(std::ostream& o,Shooter::Status_detail a){ return o<<"Shooter::Status_detail( speed:"<<a.speed<<" beam:"<<a.beam<<")"; }
std::ostream& operator<<(std::ostream& o,Shooter a){ return o<<"Shooter("<<a.estimator<<")"; }

std::ostream& operator<<(std::ostream& o,Shooter::Output out){
	o<<"Shooter::Output( ";
	if (out.mode==Talon_srx_output::Mode::SPEED) o<<"speed:"<<out.speed;
	else if(out.mode==Talon_srx_output::Mode::VOLTAGE) o<<"voltage:"<<out.voltage;
	return o<<" mode:"<<out.mode<<")";
}

bool operator==(Shooter::Input a,Shooter::Input b){ return a.speed==b.speed && a.beam==b.beam; }
bool operator!=(Shooter::Input a,Shooter::Input b){ return !(a==b); }
bool operator<(Shooter::Input a,Shooter::Input b){
	if(a.speed<b.speed) return true;
	if(b.speed<a.speed) return false;
	if(a.enabled<b.enabled) return true;
	if(b.enabled<a.enabled) return false;
	return a.beam && !b.beam;
} 

bool operator<(Shooter::Status_detail a,Shooter::Status_detail b){
	if(a.speed<b.speed) return true;
	if(b.speed<a.speed) return false;
	return a.beam && !b.beam;
}

bool operator==(Shooter::Goal a,Shooter::Goal b){ return a.mode==b.mode && a.type==b.type && a.percentage==b.percentage; }
bool operator!=(Shooter::Goal a,Shooter::Goal b){ return !(a==b); }
bool operator<(Shooter::Goal a,Shooter::Goal b){
	if(a.type<b.type) return true;
	if(b.type<a.type) return false;
	if(a.mode<b.mode) return true;
	if(b.mode<a.mode) return false;
	return a.percentage<b.percentage;
}

bool operator==(Shooter::Output a,Shooter::Output b){ return a.speed==b.speed && a.mode==b.mode; }
bool operator!=(Shooter::Output a,Shooter::Output b){ return !(a==b); }
bool operator<(Shooter::Output a,Shooter::Output b){
	if(a.speed<b.speed) return true;
	if(b.speed<a.speed) return false;
	return a.mode<b.mode;
}

bool operator==(Shooter::Status_detail a,Shooter::Status_detail b){ return (a.speed==b.speed && a.beam==b.beam); }
bool operator!=(Shooter::Status_detail a,Shooter::Status_detail b){ return !(a==b); }

bool operator<(Shooter::Input_reader,Shooter::Input_reader){ return false; }
bool operator==(Shooter::Input_reader,Shooter::Input_reader){ return true; }

bool operator==(Shooter::Estimator a,Shooter::Estimator b){ return a.last==b.last && a.last_output==b.last_output && a.speed_up_timer==b.speed_up_timer; }
bool operator!=(Shooter::Estimator a,Shooter::Estimator b){ return !(a==b); }

bool operator==(Shooter::Output_applicator,Shooter::Output_applicator){ return true; }

bool operator==(Shooter a,Shooter b){ return (a.input_reader==b.input_reader && a.estimator==b.estimator && a.output_applicator==b.output_applicator); }
bool operator!=(Shooter a,Shooter b){ return !(a==b); }

void Shooter::Goal::operator()(Shooter::Goal::Mode const& m){
	mode=m;
}

Shooter::Input Shooter::Input_reader::operator()(Robot_inputs r)const{
	return {r.talon_srx[SHOOTER_WHEEL_LOC].velocity,(r.digital_io.in[BEAM_SENSOR_DIO]==Digital_in::_1),r.robot_mode.enabled};
}
Robot_inputs Shooter::Input_reader::operator()(Robot_inputs r,Shooter::Input in)const{
	r.talon_srx[SHOOTER_WHEEL_LOC].velocity = in.speed;
	r.digital_io.in[BEAM_SENSOR_DIO]=(in.beam? Digital_in::_1 : Digital_in::_0);
	r.robot_mode.enabled=in.enabled;
	return r;
}

Shooter::Output Shooter::Output_applicator::operator()(Robot_outputs r)const{
	Shooter::Output out;
	out.mode=r.talon_srx[SHOOTER_WHEEL_LOC].mode;
	out.voltage=r.talon_srx[SHOOTER_WHEEL_LOC].power_level;
	out.speed= r.talon_srx[SHOOTER_WHEEL_LOC].speed;
	return out;
}

Robot_outputs Shooter::Output_applicator::operator()(Robot_outputs r,Shooter::Output out)const{ 
	r.talon_srx[SHOOTER_WHEEL_LOC].mode=out.mode;
	r.talon_srx[SHOOTER_WHEEL_LOC].power_level = out.voltage;
	r.talon_srx[SHOOTER_WHEEL_LOC].speed = out.speed;
	return r;
}

Shooter::Status_detail Shooter::Estimator::get()const{
	return last;
}

void Shooter::Estimator::update(Time time,Shooter::Input in,Shooter::Output output){
	last.speed=in.speed;
	if(output.mode==Talon_srx_output::Mode::VOLTAGE){
		static const float SPEED_UP_TIME=4;
		if(output!=last_output) speed_up_timer.set(SPEED_UP_TIME);
		speed_up_timer.update(time,in.enabled);
		if(speed_up_timer.done()){
			last.speed=[&]{
				if(output.voltage==0.0) return 0.0;
				if(output.voltage==-.5) return GROUND_RPM;
				if(output.voltage==-1.0) return CLIMB_RPM;
				if(output.voltage==1.0) return FREE_SPIN_RPM;
				assert(0);
			}();
		}
	} else if(output.mode==Talon_srx_output::Mode::SPEED){
		
	}
	last.beam=in.beam;
	last_output=output;
} 

std::set<Shooter::Input> examples(Shooter::Input*){
	return {
		{true,true,true},
		{true,true,false},
		{true,false,true},
		{true,false,false},
		{false,true,true},
		{false,true,false},
		{false,false,true},
		{false,false,false}
	}; 
}
std::set<Shooter::Goal> examples(Shooter::Goal*){
	std::set<Shooter::Goal> s;
	#define X(name) s.insert({Shooter::Goal::Mode::VOLTAGE,Shooter::Goal::Type::name,1.0});
	SHOOTER_TYPES
	#undef X
	#define X(name) s.insert({Shooter::Goal::Mode::SPEED_AUTO,Shooter::Goal::Type::name,1.0});
	SHOOTER_TYPES
	#undef X
	#define X(NAME) s.insert({Shooter::Goal::Mode::SPEED_MANUAL,Shooter::Goal::Type::NAME,1.0});
	SHOOTER_TYPES
	#undef X
	return s;
}
std::set<Shooter::Status_detail> examples(Shooter::Status_detail*){
	std::set<Shooter::Status_detail> s;
	bool beam=false;
	for(unsigned int i=0; i<2; i++){
		s.insert({0,beam});
		s.insert({GROUND_RPM,beam});
		s.insert({CLIMB_RPM,beam});
		s.insert({FREE_SPIN_RPM,beam});
		beam=true;
	}
	return s;
}

std::set<Shooter::Output> examples(Shooter::Output*){
	std::set<Shooter::Output> s;
	#define X(POWER) s.insert({0,POWER,Talon_srx_output::Mode::VOLTAGE});
	X(0.0) X(-.5) X(-1.0) X(1.0)
	#undef X
	#define X(RPM) s.insert({RPM,0,Talon_srx_output::Mode::SPEED});
	X(GROUND_RPM) X(0.0) X(CLIMB_RPM) X(FREE_SPIN_RPM)
	#undef X
	return s;
}

Shooter::Output control(Shooter::Status_detail, Shooter::Goal goal){
	Shooter::Output out;
	out.mode=[&]{
		switch(goal.mode){
			case Shooter::Goal::Mode::VOLTAGE:
				return Talon_srx_output::Mode::VOLTAGE;
			case Shooter::Goal::Mode::SPEED_AUTO:
			case Shooter::Goal::Mode::SPEED_MANUAL:
				return Talon_srx_output::Mode::SPEED;
			default: assert(0);
		}
	}();
	switch(goal.mode) {
		case Shooter::Goal::Mode::SPEED_MANUAL:
		case Shooter::Goal::Mode::SPEED_AUTO:
			out.speed=[&]{
				switch(goal.type){
					case Shooter::Goal::Type::STOP: return 0.0;
					case Shooter::Goal::Type::GROUND_SHOT: return GROUND_RPM;
					case Shooter::Goal::Type::CLIMB_SHOT: return CLIMB_RPM;
					case Shooter::Goal::Type::X: return FREE_SPIN_RPM;
					default: assert(0);
				}
			}();
			out.voltage=0;
			break;
		case Shooter::Goal::Mode::VOLTAGE:
			out.voltage=[&]{
				switch(goal.type){
					case Shooter::Goal::Type::STOP: return 0.0;
					case Shooter::Goal::Type::GROUND_SHOT: return -.5;
					case Shooter::Goal::Type::CLIMB_SHOT: return -1.0;
					case Shooter::Goal::Type::X: return 1.0;
					default: assert(0);
				}
			}();
			break;
		default: assert(0);
	}
	return out;
}
Shooter::Status status(Shooter::Status_detail a){
	return a;
}

bool approx_speed(double curr, double target) {
	const double PERCENT_TOLERANCE=.02;
	return (std::fabs(curr - target) < std::fabs(target)*PERCENT_TOLERANCE);
}

bool ready(Shooter::Status status,Shooter::Goal goal){	
	switch(goal.type){
		case Shooter::Goal::Type::STOP: return approx_speed(status.speed, 0);
		case Shooter::Goal::Type::GROUND_SHOT: return approx_speed(status.speed, GROUND_RPM);
		case Shooter::Goal::Type::CLIMB_SHOT: return approx_speed(status.speed, CLIMB_RPM);
		case Shooter::Goal::Type::X: return true;
		default: assert(0);
	}
}

#ifdef SHOOTER_TEST
#include "formal.h"

int main(){
	Shooter a;
	tester(a);
}

#endif

