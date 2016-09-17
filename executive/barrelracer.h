#ifndef BARRELRACER_H
#define BARRELRACER_H

#include "executive.h"
#include "../util/motion_profile.h"

struct Auto_br_straightaway : public Executive_impl<Auto_br_straightaway>{
	unsigned int br_step;
	Countdown_timer in_br_range;
	Motion_profile motion_profile;
	std::pair<int,int> initial_encoders;
	
	Auto_br_straightaway(unsigned int bs,std::pair<int,int> ie):br_step(bs),initial_encoders(ie){}

	Executive next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_br_straightaway const&)const;
};

struct Auto_br_initialturn : public Executive_impl<Auto_br_initialturn>{
	unsigned int br_step;
	Motion_profile motion_profile;
	std::pair<int,int> initial_encoders;

	Auto_br_initialturn(unsigned int bs,std::pair<int,int> ie):br_step(bs),initial_encoders(ie){}

	Executive next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_br_initialturn const&)const;
};

struct Auto_br_side : public Executive_impl<Auto_br_side>{
	unsigned int br_step;
	Motion_profile motion_profile;
	std::pair<int,int> initial_encoders;

	Auto_br_side(unsigned int bs,std::pair<int,int> ie):br_step(bs),initial_encoders(ie){}
	
	Executive next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_br_side const&)const;
};

struct Auto_br_sideturn : public Executive_impl<Auto_br_sideturn>{
	unsigned int br_step;
	Motion_profile motion_profile;
	std::pair<int,int> initial_encoders;

	Auto_br_sideturn(unsigned int bs,std::pair<int,int> ie):br_step(bs),initial_encoders(ie){}

	Executive next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_br_sideturn const&)const;
};

struct Auto_br_endturn : public Executive_impl<Auto_br_endturn>{
	unsigned int br_step;
	Motion_profile motion_profile;
	std::pair<int,int> initial_encoders;

	Auto_br_endturn(unsigned int bs,std::pair<int,int> ie):br_step(bs),initial_encoders(ie){}

	Executive next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_br_endturn const&)const;
};

#endif