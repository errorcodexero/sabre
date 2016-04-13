#include "talon_srx_control.h"
#include "WPILib.h"
#include "util/util.h"
#include <cmath>

using namespace std;

Talon_srx_control::Talon_srx_control():talon(NULL),since_query(0),mode(Mode::INIT){}

Talon_srx_control::Talon_srx_control(int CANBusAddress):talon(NULL),since_query(0),mode(Mode::INIT) {
	init(CANBusAddress);
}

Talon_srx_control::~Talon_srx_control(){
	delete talon;
}

void Talon_srx_control::init(int CANBusAddress){
	assert(!talon);
	assert(mode==Mode::INIT);
	talon = new CANTalon(CANBusAddress);
	assert(talon);
	talon->SetSafetyEnabled(false);
	mode = Mode::VOLTAGE;
}

ostream& operator<<(ostream& o,Talon_srx_control::Mode a){
	#define X(NAME) if(a==Talon_srx_control::Mode::NAME) return o<<""#NAME;
	TALON_SRX_MODES
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o,Talon_srx_control a){
	return o<<"Talon_srx_control( mode:"<<a.mode<<" out:"<<a.out<</*" last_out:"<<a.last_out<<*/" init:"<<!!a.talon<<" since_query:"<<a.since_query<<" in:"<<a.in<<")";
}

bool pid_approx(Talon_srx_output::PID_coefficients a,Talon_srx_output::PID_coefficients b){
	const float TOLERANCE=.001;
	return fabs(a.p-b.p)<TOLERANCE && fabs(a.i-b.i)<TOLERANCE && fabs(a.d-b.d)<TOLERANCE;
}

void Talon_srx_control::set(Talon_srx_output a, bool enable) {
	static const float EXPIRATION=2.0;
	assert(mode!=Mode::INIT);
	if(!enable){
		if(mode!=Talon_srx_control::Mode::DISABLE){ 
			talon->Set(0);
			talon->SetSafetyEnabled(false);
			talon->Disable();
			mode=Talon_srx_control::Mode::DISABLE;
		}
		return;
	}
	if(a.mode==Talon_srx_output::Mode::VOLTAGE){
		if(mode!=Talon_srx_control::Mode::VOLTAGE){
			talon->SetControlMode(CANTalon::kVoltage);
			talon->EnableControl();
			talon->SetExpiration(EXPIRATION);
			talon->SetSafetyEnabled(true);
			talon->Set(a.power_level);
			out=a;
			mode=Talon_srx_control::Mode::VOLTAGE;
		} else if((a.power_level!=out.power_level || since_query==1) /*&& out!=last_out*/){
			talon->Set(a.power_level);
			out.power_level=a.power_level;
		}	
	} else if(a.mode==Talon_srx_output::Mode::SPEED){
		double error = in.velocity-a.speed;
		double power = clip(error);
		if(mode!=Talon_srx_control::Mode::SPEED || !pid_approx(out.pid,a.pid)){
			talon->SetControlMode(CANTalon::kSpeed);
			talon->SetPID(a.pid.p,a.pid.i,a.pid.d);	
			talon->EnableControl();
			talon->ConfigEncoderCodesPerRev(1);
			talon->SetExpiration(EXPIRATION);
			talon->SetSafetyEnabled(true);
			talon->Set(power);
			out=a;
			mode=Talon_srx_control::Mode::SPEED;
		} else if((a.speed!=out.speed || since_query==1) /*&& out!=last_out*/){ 
			talon->Set(power);
			out.speed=a.speed;
		}
	}
	//last_out=out;
}

Talon_srx_input Talon_srx_control::get(){
	if(since_query>20){
		switch(mode){
			case Talon_srx_control::Mode::VOLTAGE:
				in.current=talon->GetBusVoltage();
				break;
			case Talon_srx_control::Mode::SPEED:
				in.velocity=talon->GetSpeed();
				break;
			default: break;
		}
	}
	since_query++;
	return in;
}

Talon_srx_controls::Talon_srx_controls():init_(false){}

void Talon_srx_controls::init(){
	if(!init_){
		for(unsigned int i=0; i<Robot_outputs::TALON_SRX_OUTPUTS; i++){
			talons[i].init(i);
		}
		init_=true;
	}
}

void Talon_srx_controls::set(Checked_array<Talon_srx_output,Robot_outputs::TALON_SRX_OUTPUTS> const& a,Checked_array<bool,Robot_outputs::TALON_SRX_OUTPUTS> const& enable){
	init();
	for(unsigned int i=0; i<Robot_outputs::TALON_SRX_OUTPUTS; i++){
		talons[i].set(a[i],enable[i]);
	}
}

array<Talon_srx_input,Robot_inputs::TALON_SRX_INPUTS> Talon_srx_controls::get(){
	init();
	
	array<Talon_srx_input,Robot_inputs::TALON_SRX_INPUTS> inputs;
	for(unsigned int i=0; i<Robot_inputs::TALON_SRX_INPUTS; i++){
		inputs[i]=talons[i].get();
	}
	return inputs;
}

ostream& operator<<(ostream& o,Talon_srx_controls const& t){
	o<<"Talon_srx_controls(";
	o<< "init:"<<t.init_;
	//o<<" talons("<<t.talons;
	return o<<"))";
}

