#ifndef LOW_BAR_WALL_LOW_SCORE_H
#define LOW_BAR_WALL_LOW_SCORE_H

#include "mode.h"

struct Auto_lbwls_wall: public Mode_impl<Auto_lbwls_wall>{
	Mode next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_lbwls_wall const&)const;
};

struct Auto_lbwls_mup: Mode_impl<Auto_lbwls_mup>{
	bool topready=0;

	Mode next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_lbwls_mup const&)const;
};

struct Auto_lbwls_rotate: Mode_impl<Auto_lbwls_rotate>{
	Mode next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_lbwls_rotate const&)const;
};

struct Auto_lbwls_tower: Mode_impl<Auto_lbwls_tower>{
	Mode next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_lbwls_tower const&)const;
};

struct Auto_lbwls_eject: Mode_impl<Auto_lbwls_eject>{
	Mode next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_lbwls_eject const&)const;
};

struct Auto_lbwls_back: Mode_impl<Auto_lbwls_back>{
	Mode next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_lbwls_back const&)const;
};

struct Auto_lbwls_c: Mode_impl<Auto_lbwls_c>{
	Mode next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_lbwls_c const&)const;
};

struct Auto_lbwls_br: Mode_impl<Auto_lbwls_br>{
	Mode next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_lbwls_br const&)const;
};

#endif
