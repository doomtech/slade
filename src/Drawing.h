
#ifndef __DRAWING_H__
#define __DRAWING_H__

namespace Drawing {
	// Basic drawing
	void drawLine(fpoint2_t start, fpoint2_t end);
	void drawLine(double x1, double y1, double x2, double y2);
	void drawRect(fpoint2_t tl, fpoint2_t br);
	void drawRect(double x1, double y1, double x2, double y2);
	void drawFilledRect(fpoint2_t tl, fpoint2_t br);
	void drawFilledRect(double x1, double y1, double x2, double y2);

	// Specific
	void drawHud(bool statusbar, bool center, bool wide);
}

#endif//__DRAWING_H__
