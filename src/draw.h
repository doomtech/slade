
#ifndef __DRAW_H__
#define __DRAW_H__

#include "structs.h"
#include "textures.h"

void set_gl_colour(rgba_t col);
void draw_point(int x, int y, int size, rgba_t col);
void draw_line(rect_t rect, rgba_t col, bool aa, bool side_indicator = false, float width = 1.0f);
void draw_rect(rect_t rect, rgba_t col, bool fill);
void draw_texture_scale(rect_t rect, Texture* tex, rgba_t col = COL_WHITE, float scalef = 1.0f);
void draw_texture_scale(rect_t rect, string texname = "_notex", int textype = 0, rgba_t col = COL_WHITE, float scalef = 1.0f);
void draw_texture(int x, int y, int width = -1, int height = -1, string texname = "_notex", int textype = 0, rgba_t col = COL_WHITE);
void draw_text(int x, int y, rgba_t colour, BYTE alignment, const char* text, ...);
void draw_text_scale(int x, int y, rgba_t colour, BYTE alignment, int scale, const char* text, ...);

#endif //__DRAW_H__
