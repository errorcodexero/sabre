#ifndef AUTO_STOP_H
#define AUTO_STOP_H

#include "mode.h"

struct Auto_stop: public Mode_impl<Auto_stop>{
	Mode next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_stop const&)const;
};

#endif
