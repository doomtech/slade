
#ifndef __DRAWING_H__
#define __DRAWING_H__

class GLTexture;
namespace Drawing {
	// Basic drawing
	void drawLine(fpoint2_t start, fpoint2_t end);
	void drawLine(double x1, double y1, double x2, double y2);
	void drawRect(fpoint2_t tl, fpoint2_t br);
	void drawRect(double x1, double y1, double x2, double y2);
	void drawFilledRect(fpoint2_t tl, fpoint2_t br);
	void drawFilledRect(double x1, double y1, double x2, double y2);

	// Texture drawing
	void drawTextureWithin(GLTexture* tex, double x1, double y1, double x2, double y2, double padding, bool upscale = false);

	// Specific
	void drawHud(bool statusbar, bool center, bool wide);
}

#endif//__DRAWING_H__
