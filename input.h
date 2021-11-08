#ifndef BUFFDOG_INPUT
#define BUFFDOG_INPUT

struct mouse_state {
	double motion_x;
	double motion_y;
	int pos_x;
	int pos_y;
};

struct buttons_state {
	bool forward;
	bool reverse;
	bool left;
	bool right;
	bool rise;
	bool descend;
	bool sprint;
	bool action1; // m1
	bool action2;
	bool action3;
	bool action4;
};

struct InputState {
	mouse_state mouse;
	buttons_state buttons;
};

#endif
