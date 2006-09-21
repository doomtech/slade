
#ifndef __EDIT_H__
#define __EDIT_H__

class Vertex;

// Editor states. Can only be changed if the current state is 0 (normal).
#define STATE_NORMAL		0
#define STATE_LINEDRAW		1
#define STATE_MOVING		2
#define STATE_PASTE			3
#define STATE_THINGANGLE	4
#define STATE_MAPPAN		5
#define STATE_DIALOG		6
#define STATE_3DMODE		7
#define STATE_SHAPEDRAW		8

#define STATE_OTHER			255

bool change_state(BYTE new_state = 0);
bool state(BYTE state_test = 0);
void increase_grid();
void decrease_grid();
int snap_to_grid(double pos);

void line_flip(bool verts, bool sides);

Vertex* create_vertex(point2_t mouse);
void create_line(bool close);
void create_sector();
void create_thing(point2_t mouse);

void check_textures();

#endif //__EDIT_H__
