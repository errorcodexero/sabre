#ifndef AUTO_STATICTWO_H
#define AUTO_STATICTWO_H

#include "mode.h"

struct Auto_statictwo: public Mode_impl<Auto_statictwo>{
	Mode next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_statictwo const&)const;
};

#endif
