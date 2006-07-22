
bool sector_create(point2_t point, bool* front_sides = NULL, bool* back_sides = NULL);

void ldraw_addpoint(bool nearest_vert);
void ldraw_draw_lines(point2_t mouse);
void ldraw_removepoint();
void ldraw_drawrect(point2_t mouse, bool square);
