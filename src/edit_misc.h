
#include "dm_sector.h"

bool sector_create(point2_t point, vector<Sector*> &new_sectors, bool* front_sides = NULL, bool* back_sides = NULL);


void ldraw_addpoint(bool nearest_vert);
void ldraw_draw_lines(point2_t mouse);
void ldraw_removepoint();
void ldraw_drawrect(point2_t mouse, bool square);
void ldraw_drawcircle(point2_t mouse, int sides, bool circle);


int get_side_sector(int line, int side);


void line_paint_tex(int line, bool front, string otex, string ntex, vector<int> &processed_lines);
void line_auto_align_x(int line, int offset, bool front, string tex, int texwidth, vector<int> &processed_lines);
void line_extrude(int amount);
void line_align_x();
void line_correct_references(int line = -1, bool backup = true);
void line_split(int splits = 2);
void line_flip(bool verts, bool sides);
void line_split_at(point2_t pos);


void sector_create_stairs(int floor_step, int ceil_step);
void sector_create_door(string texture);
void sector_merge(bool remove_lines);
void sector_create(point2_t point);
void sector_light_gradient(int step);
void sector_paint_tex(int sector, string tex, bool floor, vector<int> &sectors);
void sector_changelight(int amount);
void sector_changeheight(bool floor, int amount);


void vert_rotate(double angle);
void vert_mirror(bool mirror_x, bool mirror_y);


void thing_rotate(double angle, bool rot_angles);
void thing_mirror(bool mirror_x, bool mirror_y, bool angles);
void thing_setquickangle();
