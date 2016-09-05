#include "low_bar_wall_low_score.h"

using namespace std;

unique_ptr<Mode> Auto_lbwls_wall::next_mode(Next_mode_info){
	return make_unique<Auto_lbwls_wall>();
}

Toplevel::Goal Auto_lbwls_wall::run(Run_info){
	return {};
}

unique_ptr<Mode> Auto_lbwls_mup::next_mode(Next_mode_info){
	return make_unique<Auto_lbwls_mup>();
}

Toplevel::Goal Auto_lbwls_mup::run(Run_info){
	return {};
}

unique_ptr<Mode> Auto_lbwls_rotate::next_mode(Next_mode_info){
	return make_unique<Auto_lbwls_rotate>();
}

Toplevel::Goal Auto_lbwls_rotate::run(Run_info){
	return {};
}

unique_ptr<Mode> Auto_lbwls_tower::next_mode(Next_mode_info){
	return make_unique<Auto_lbwls_tower>();
}

Toplevel::Goal Auto_lbwls_tower::run(Run_info){
	return {};
}

unique_ptr<Mode> Auto_lbwls_eject::next_mode(Next_mode_info){
	return make_unique<Auto_lbwls_eject>();
}

Toplevel::Goal Auto_lbwls_eject::run(Run_info){
	return {};
}

unique_ptr<Mode> Auto_lbwls_back::next_mode(Next_mode_info){
	return make_unique<Auto_lbwls_back>();
}

Toplevel::Goal Auto_lbwls_back::run(Run_info){
	return {};
}

unique_ptr<Mode> Auto_lbwls_c::next_mode(Next_mode_info){
	return make_unique<Auto_lbwls_c>();
}

Toplevel::Goal Auto_lbwls_c::run(Run_info){
	return {};
}

unique_ptr<Mode> Auto_lbwls_br::next_mode(Next_mode_info){
	return make_unique<Auto_lbwls_br>();
}

Toplevel::Goal Auto_lbwls_br::run(Run_info){
	return {};
}


#ifdef LOW_BAR_WALL_LOW_SCORE_TEST
int main(){
	
}
#endif 
