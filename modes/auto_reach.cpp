#include "auto_reach.h"
#include "teleop.h"
#include "auto_stop.h"

using namespace std;

Mode Auto_reach::next_mode(Next_mode_info info){
	if(!info.autonomous) return Mode{Teleop()};
	if(info.since_switch > .8) return Mode{Auto_stop()};
	return Mode{Auto_reach()};
}

Toplevel::Goal Auto_reach::run(Run_info){
	Toplevel::Goal goals;
	goals.drive.left=.45;
	goals.drive.right=.45;
	
	return goals;
}

bool Auto_reach::operator==(Auto_reach const&)const{ return 1; }

#ifdef AUTO_REACH_TEST
int main(){
	Auto_reach a;
	test_mode(a);
}
#endif 
