#include "delay.h"

#include "teleop.h" 
#include "auto_null.h"
#include "auto_reach.h"
#include "auto_statictwo.h"
#include "auto_static.h"
#include "portcullis.h"
#include "cheval.h"
#include "low_bar_low_score.h"
#include "low_bar_wall_low_score.h"
#include "low_bar_wall_high_score.h"
#include "barrelracer.h"

using namespace std;

Mode get_auto1(Panel const& panel){
	if (panel.in_use) {
		switch(panel.auto_mode){ 
			case Panel::Auto_mode::NOTHING:
				return Mode{Auto_null()};
			case Panel::Auto_mode::REACH:
				return Mode{Auto_reach()};
			case Panel::Auto_mode::STATICS:
				return Mode{Auto_statictwo()};
			case Panel::Auto_mode::STATICF:
				return Mode{Auto_static()};
			case Panel::Auto_mode::PORTCULLIS:
				return Mode{Auto_portcullis()};
			case Panel::Auto_mode::CHEVAL:
				return Mode{Auto_cheval_pos()};
			case Panel::Auto_mode::LBLS:
				return Mode{Auto_lbls_cross_lb()};
			case Panel::Auto_mode::LBWLS:	
				return Mode{Auto_lbwls_wall()};
			case Panel::Auto_mode::LBWHS:
				return Mode{Auto_lbwhs_wall()};
			case Panel::Auto_mode::S:
				return Mode{Auto_lbwhs_prep()};
			case Panel::Auto_mode::BR:
				//FIXME: For now, just choosing some number to put in.
				return Mode{Auto_br_straightaway(0)};
			default: assert(0);
		}
	}
	return Mode{Delay()};
}

Mode Delay::next_mode(Next_mode_info info){
	if(!info.autonomous) return Mode{Teleop()};
	if(info.since_switch > (info.panel.speed_dial+1)*5 || info.since_switch > 8) return get_auto1(info.panel);
	return Mode{Delay()};
}

Toplevel::Goal Delay::run(Run_info){
	
	return {};
}

bool Delay::operator==(Delay const&)const{ return 1; }

#ifdef DELAY_TEST
int main(){
	Delay a;
	test_mode(a);
}
#endif 
