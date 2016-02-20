
#include "tilt.h"
#include <stdlib.h>
#include <cmath>
#include <string>
#include <sstream>
#include <fstream> 
#include <vector> 

enum Positions{UP,LEVEL,LOW,DOWN,POSITIONS};
std::array<float,Positions::POSITIONS> positions={1.4,2.4,2.79,3.2};//in volts
static const std::array<std::string,Positions::POSITIONS> POSITION_NAMES={"UP","LEVEL","LOW","DOWN"};	
static const std::string POSITIONS_PATH="/home/lvuser/";
static const std::string POSITIONS_FILE=[&]{
	std::string s;
	#ifndef TILT_TEST
	s=POSITIONS_PATH;
	#endif
	return s.append("tilt_positions.txt");
}();

#define VOLTS_PER_DEGREE .03// (in volts/degree) //Assumed for now

#define ANGLE_TOLERANCE 10//in degrees, may want to change later

#define TILT_PDB_LOC 8
#define TILT_POT_LOC 0
#define TILT_LIM_LOC 9
#define TILT_ADDRESS 4

#define nyi { std::cout<<"\nnyi "<<__LINE__<<"\n"; exit(44); }

float volts_to_degrees(float f){
	return f/VOLTS_PER_DEGREE;
}

float degrees_to_volts(float f){
	return f*VOLTS_PER_DEGREE;
}

Tilt::Status_detail::Status_detail(): 
	reached_ends(std::make_pair(0,0)),
	stalled(0),
	type_(Tilt::Status_detail::Type::MID),
	angle(0),
	pot_value_(0)
{}

Tilt::Goal::Goal():mode_(Tilt::Goal::Mode::STOP),angle_min(0),angle_target(0),angle_max(0){}

Robot_inputs Tilt::Input_reader::operator()(Robot_inputs r,Tilt::Input in)const{
	r.current[TILT_PDB_LOC]=in.current;
	r.analog[TILT_POT_LOC]=in.pot_value;
	r.digital_io.in[TILT_LIM_LOC]=in.top ? Digital_in::_0 : Digital_in::_1;
	return r;
}

Tilt::Input Tilt::Input_reader::operator()(Robot_inputs r)const{
	return {r.analog[TILT_POT_LOC],r.current[TILT_PDB_LOC],r.digital_io.in[TILT_LIM_LOC]==Digital_in::_0};
}

Tilt::Output Tilt::Output_applicator::operator()(Robot_outputs r)const{
	return r.pwm[TILT_ADDRESS];
}

Robot_outputs Tilt::Output_applicator::operator()(Robot_outputs r, Tilt::Output out)const{
	r.pwm[TILT_ADDRESS]=out;
	return r;
}

Tilt::Goal::Mode Tilt::Goal::mode()const{
	return mode_;
}

Tilt::Estimator::Estimator():
	last(Tilt::Status_detail::error()),
	timer_start_angle(0)
{}

std::array<double,3> Tilt::Goal::angle()const{
	assert(mode_==Tilt::Goal::Mode::GO_TO_ANGLE);
	return std::array<double,3>{angle_min,angle_target,angle_max};
}

Tilt::Goal Tilt::Goal::up(){
	Tilt::Goal a;
	a.mode_=Tilt::Goal::Mode::UP;
	return a;
}

Tilt::Goal Tilt::Goal::go_to_angle(std::array<double,3> angles){
	//assert(angles[0]>=(volts_to_degrees(positions[Positions::UP]-positions[Positions::UP])) && angles[2]<=(volts_to_degrees(positions[Positions::DOWN]-positions[Positions::UP])));
	Tilt::Goal a;
	a.mode_=Tilt::Goal::Mode::GO_TO_ANGLE;
	a.angle_min=angles[0];
	a.angle_target=angles[1];
	a.angle_max=angles[2];
	return a;
}

Tilt::Goal Tilt::Goal::down(){
	Tilt::Goal a;
	a.mode_=Tilt::Goal::Mode::DOWN;
	return a;
}

Tilt::Goal Tilt::Goal::stop(){
	Tilt::Goal a;
	a.mode_=Tilt::Goal::Mode::STOP;
	return a;
}

Tilt::Goal Tilt::Goal::low(){
	return Tilt::Goal::go_to_angle(make_tolerances(volts_to_degrees(positions[Positions::LOW])));
}

Tilt::Goal Tilt::Goal::level(){
	return Tilt::Goal::go_to_angle(make_tolerances(volts_to_degrees(positions[Positions::LEVEL])));
}

Tilt::Status_detail::Type Tilt::Status_detail::type()const{
	return type_;
}

double Tilt::Status_detail::get_angle()const{
	return angle;
}

float Tilt::Status_detail::pot_value()const{
	return pot_value_;
}

Tilt::Status_detail Tilt::Status_detail::top(){
	Tilt::Status_detail a;
	a.type_=Tilt::Status_detail::Type::TOP;
	return a;
}

Tilt::Status_detail Tilt::Status_detail::mid(double d){
	Tilt::Status_detail a;
	a.type_=Tilt::Status_detail::Type::MID;
	a.angle=d;
	a.pot_value_=degrees_to_volts(d);
	return a;
}

Tilt::Status_detail Tilt::Status_detail::bottom(){
	Tilt::Status_detail a;
	a.type_=Tilt::Status_detail::Type::BOTTOM;
	return a;
}

Tilt::Status_detail Tilt::Status_detail::error(){
	Tilt::Status_detail a;
	a.type_=Tilt::Status_detail::Type::ERRORS;
	return a;
}

std::ostream& operator<<(std::ostream& o, Tilt::Status_detail::Type a){
	#define X(name) if(a==Tilt::Status_detail::Type::name) return o<<""#name;
	TILT_STATUS_DETAIL_TYPES
	#undef X
	nyi
}

std::ostream& operator<<(std::ostream& o, Tilt::Goal::Mode a){
	#define X(name) if(a==Tilt::Goal::Mode::name) return o<<"Tilt::Goal("#name")";
	TILT_GOAL_MODES
	#undef X
	nyi
}

std::ostream& operator<<(std::ostream& o, Tilt::Status_detail a){ 
	o<<"Tilt::Status_detail(";
	o<<" stalled:"<<a.stalled;
	o<<" reached_ends:"<<a.reached_ends;
	o<<" type:"<<a.type();
	if(a.type()==Tilt::Status_detail::Type::MID)o<<"("<<a.get_angle()<<" "<<a.pot_value()<<")";
	return o<<")";
}

std::ostream& operator<<(std::ostream& o, Tilt::Goal a){ 
	o<<"Tilt::Goal(";
	o<<" mode:"<<a.mode();
	if(a.mode()==Tilt::Goal::Mode::GO_TO_ANGLE)o<<"("<<a.angle()<<")";
	return o<<")";
}

std::ostream& operator<<(std::ostream& o, Tilt::Output_applicator){ return o<<"Tilt::Output_applicator()";} 
std::ostream& operator<<(std::ostream& o, Tilt::Input_reader){ return o<<"Tilt::Input_reader()";}
std::ostream& operator<<(std::ostream& o, Tilt::Estimator a){ return o<<"Tilt::Estimator( last:"<<a.get()<<" stall_timer:"<<a.stall_timer<<" timer_start_angle:"<<a.timer_start_angle<<")";} 
 
std::ostream& operator<<(std::ostream& o, Tilt a){
	o<<"Tilt(";
	o<<" "<<a.output_applicator;
	o<<" "<<a.input_reader;
	o<<" "<<a.estimator;
	return o<<")";
}

#define CMP(name) if(a.name<b.name) return 1; if(b.name<a.name) return 0;

bool operator==(Tilt::Input const& a,Tilt::Input const& b){
	return a.pot_value==b.pot_value && a.current==b.current && a.top==b.top;	
}
bool operator!=(Tilt::Input const& a,Tilt::Input const& b){ return !(a==b); }
bool operator<(Tilt::Input const& a,Tilt::Input const& b){
	CMP(pot_value)
	CMP(current)
	return !a.top && b.top;
}
std::ostream& operator<<(std::ostream& o,Tilt::Input const& a){ return o<<"Tilt::Input( pot_value:"<<a.pot_value<<" current:"<<a.current<<" top:"<<a.top<<")"; }

bool operator<(Tilt::Status_detail a, Tilt::Status_detail b){
	CMP(type())
	if(a.type()==Tilt::Status_detail::Type::MID){
		CMP(pot_value())
		CMP(get_angle())
	}
	CMP(reached_ends)
	return !a.stalled && b.stalled;
}
bool operator==(Tilt::Status_detail a,Tilt::Status_detail b){
	if(a.type()!=b.type()) return 0;
	return ((a.type()!=Tilt::Status_detail::Type::MID || (a.get_angle()==b.get_angle() && a.pot_value()==b.pot_value())) && a.reached_ends==b.reached_ends && a.stalled==b.stalled);
}
bool operator!=(Tilt::Status_detail a,Tilt::Status_detail b){ return !(a==b); }

bool operator==(Tilt::Goal a, Tilt::Goal b){ 
	return a.mode()==b.mode() && (a.mode()==Tilt::Goal::Mode::GO_TO_ANGLE ? a.angle()==b.angle() : true );
}
bool operator!=(Tilt::Goal a, Tilt::Goal b){ return !(a==b); }
bool operator<(Tilt::Goal a, Tilt::Goal b){
	if(a.mode()<b.mode()) return true;
	if(b.mode()<a.mode()) return false;
	if(a.mode()==Tilt::Goal::Mode::GO_TO_ANGLE) return a.angle()<b.angle();
	return false;
}

bool operator==(Tilt::Output_applicator,Tilt::Output_applicator){ return true; }

bool operator==(Tilt::Input_reader,Tilt::Input_reader){ return true; }

bool operator==(Tilt::Estimator a,Tilt::Estimator b){ return a.last==b.last; }
bool operator!=(Tilt::Estimator a,Tilt::Estimator b){ return !(a==b); }

bool operator==(Tilt a, Tilt b){ return (a.output_applicator==b.output_applicator && a.input_reader==b.input_reader && a.estimator==b.estimator); }
bool operator!=(Tilt a, Tilt b){ return !(a==b); }

std::set<Tilt::Input> examples(Tilt::Input*){ 
	std::set<Tilt::Input> s;
	for(unsigned int i=0; i<Positions::POSITIONS; i++){
		s.insert({positions[i],0,i==Positions::UP});
	}
	return s;
}

std::set<Tilt::Goal> examples(Tilt::Goal*){
	return {
		Tilt::Goal::down(),
		Tilt::Goal::stop(),
		Tilt::Goal::low(),
		Tilt::Goal::level(),
		Tilt::Goal::go_to_angle(make_tolerances(ANGLE_TOLERANCE)),
		Tilt::Goal::up()
	};
}

std::set<Tilt::Status_detail> examples(Tilt::Status_detail*){
	return {
		Tilt::Status_detail::top(),
		Tilt::Status_detail::mid(0),
		Tilt::Status_detail::bottom(),
		Tilt::Status_detail::error()
	};
}

std::set<Tilt::Output> examples(Tilt::Output*){ 
	return {-1,-0.93,-0.8,0,1};
}

Tilt::Output control(Tilt::Status_detail status, Tilt::Goal goal){
	const double POWER=1;//negative goes up, positive goes down
	switch(goal.mode()){
		case Tilt::Goal::Mode::UP:
			switch(status.type()){
				case Tilt::Status_detail::Type::TOP:
				case Tilt::Status_detail::Type::ERRORS:
					return 0;
				case Tilt::Status_detail::Type::BOTTOM:
				case Tilt::Status_detail::Type::MID:
					return -POWER;
				default: assert(0);
			}
		case Tilt::Goal::Mode::DOWN:
			switch(status.type()){
				case Tilt::Status_detail::Type::BOTTOM:
				case Tilt::Status_detail::Type::ERRORS:
					return 0;
				case Tilt::Status_detail::Type::TOP:
				case Tilt::Status_detail::Type::MID:
					return POWER;
				default: assert(0);
			}
		case Tilt::Goal::Mode::GO_TO_ANGLE:
			switch (status.type()) {
				case Tilt::Status_detail::Type::MID:
					{
						const double SCALE_DEGREE_TO_POWER=.01;
						if(status.get_angle()>=goal.angle()[0] && status.get_angle()<=goal.angle()[2])return 0.0;
						std::cout<<"P:"<<std::min(fabs(goal.angle()[1]-status.get_angle())*(POWER*SCALE_DEGREE_TO_POWER), POWER) * ((goal.angle()[1] > status.get_angle()) ? -1 : 1)<<"\n";
						return std::min(abs(goal.angle()[1]-status.get_angle())*(POWER*SCALE_DEGREE_TO_POWER), POWER) * ((goal.angle()[1] > status.get_angle()) ? -1 : 1);
					}
				case Tilt::Status_detail::Type::TOP:
					return POWER;
				case Tilt::Status_detail::Type::BOTTOM:
					return -POWER;
				case Tilt::Status_detail::Type::ERRORS:
					return 0.0;
				default:
					assert(0);
			}
		case Tilt::Goal::Mode::STOP: return 0.0;
		default: assert(0);
	}
}

Tilt::Status status(Tilt::Status_detail a){
	return a;
}

bool ready(Tilt::Status status, Tilt::Goal goal){
	switch(goal.mode()){
		case Tilt::Goal::Mode::UP: return status.type()==Tilt::Status_detail::Type::TOP;
		case Tilt::Goal::Mode::DOWN: return status.type()==Tilt::Status_detail::Type::BOTTOM;
		case Tilt::Goal::Mode::GO_TO_ANGLE: return status.get_angle()>goal.angle()[0] && status.get_angle()<goal.angle()[2];
		case Tilt::Goal::Mode::STOP: return 1;
		default: assert(0);
	}
}

Tilt::Status_detail Tilt::Estimator::get()const {
	return last;
}

void Tilt::Estimator::update(Time time, Tilt::Input in, Tilt::Output) {
	const float ALLOWED_TOLERANCE=degrees_to_volts(ANGLE_TOLERANCE);
	bool at_top=in.top, at_bottom=in.pot_value>=positions[Positions::DOWN]-ALLOWED_TOLERANCE;
	if(in.top){
		positions[Positions::UP]=in.pot_value;
		//tilt_learn(in.pot_value,POSITION_NAMES[Positions::UP]);
	}
	float angle=volts_to_degrees(in.pot_value-positions[Positions::UP]);
	stall_timer.update(time,true);
	if(stall_timer.done()) last.stalled=true;
	if(in.current<10 || fabs(angle-timer_start_angle)<1){//Assumed current for now
		last.stalled=0;
		stall_timer.set(1);
		timer_start_angle=angle;
	}
	if(at_top && in.pot_value<=positions[Positions::UP]+ALLOWED_TOLERANCE){
		if(at_bottom)last=Tilt::Status_detail::error();
		else last=Tilt::Status_detail::top();
	} else{
		if(at_bottom)last=Tilt::Status_detail::bottom();
		else last=Tilt::Status_detail::mid(angle);
	}
}

std::array<double,3> make_tolerances(double d){
	return {d-ANGLE_TOLERANCE,d,d+ANGLE_TOLERANCE};
}

void populate(){
	std::ifstream test(POSITIONS_FILE);
	assert(test.peek()==std::ifstream::traits_type::eof());//file is empty
	test.close();
	std::ofstream file(POSITIONS_FILE);
	for(unsigned int i=0; i<Positions::POSITIONS; i++)file<<POSITION_NAMES[i]<<":"<<positions[i]<<(i+1<Positions::POSITIONS ? "\n" : "");
	file.close();
}

void update_positions(){
	std::ifstream file(POSITIONS_FILE);
	if(file.peek()==std::ifstream::traits_type::eof()){
		file.close();
		populate();
		file.open(POSITIONS_FILE);
	}
	for(unsigned int i=0; i<Positions::POSITIONS; i++){
		bool next=false;
		std::string mode=POSITION_NAMES[i];
		while(!file.eof()){ 
			std::string edit,line; 
			std::getline(file,line); 
			for(char c:line){ 
				if(c==':' && edit==mode){ 
					std::istringstream in(line.substr(edit.size()+1));
					float value;
					in>>value;
					positions[i]=value;
					next=true;
					break; 
				} 
				edit+=c; 
			} 
			if(next)break;
		} 
	}
	file.close();
}

void tilt_learn(float const& pot_in,std::string const& mode){
	std::cout<<"\nTRYING TO LEARN\n";
	assert(mode==POSITION_NAMES[Positions::UP] || mode==POSITION_NAMES[Positions::LOW] || mode==POSITION_NAMES[Positions::LEVEL] || mode==POSITION_NAMES[Positions::DOWN]);
	std::vector<std::string> go_out;
	{
		std::ifstream file(POSITIONS_FILE);
		if(file.peek()==std::ifstream::traits_type::eof()){
			file.close();
			populate();
			file.open(POSITIONS_FILE);
		}
		while(!file.eof()){ 
			std::string edit,line; 
			std::getline(file,line); 
			for(char c:line){ 
				if(c==':' && edit==mode){ 
					std::ostringstream out;
					out<<pot_in; 
					edit+=":"+out.str(); 
					break;  
				} 
				edit+=c; 
			} 
			go_out.push_back(edit); 
		} 
		file.close();
	}
	std::ofstream file(POSITIONS_FILE);
	for(unsigned int i=0; i<go_out.size(); i++)file<<go_out[i]<<(i+1<go_out.size() ? "\n" : "");
	file.close();
	update_positions();
}

#ifdef TILT_TEST
#include "formal.h"

int main(){
	update_positions();
	Tilt a;
	tester(a);
}

#endif

