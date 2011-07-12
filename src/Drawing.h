
#ifndef __DRAWING_H__
#define __DRAWING_H__

#ifdef USE_SFML_RENDERWINDOW
class sf::RenderWindow;
#endif

class GLTexture;
namespace Drawing {
	enum {
		// Text fonts
		FONT_SMALL = 0,
		FONT_LARGE = 1,

		// Text alignment
		ALIGN_LEFT = 0,
		ALIGN_RIGHT = 1,
		ALIGN_CENTER = 2,
	};

	// Initialisation
	void initFonts();

	// Basic drawing
	void drawLine(fpoint2_t start, fpoint2_t end);
	void drawLine(double x1, double y1, double x2, double y2);
	void drawRect(fpoint2_t tl, fpoint2_t br);
	void drawRect(double x1, double y1, double x2, double y2);
	void drawFilledRect(fpoint2_t tl, fpoint2_t br);
	void drawFilledRect(double x1, double y1, double x2, double y2);

	// Texture drawing
	void drawTextureWithin(GLTexture* tex, double x1, double y1, double x2, double y2, double padding, bool upscale = false);

	// Text drawing
	void drawText(string text, int x = 0, int y = 0, rgba_t colour = COL_WHITE, int font = FONT_SMALL, int alignment = ALIGN_LEFT, frect_t* bounds = NULL);

	// Specific
	void drawHud(bool statusbar, bool center, bool wide);

	// Implementation-specific
#ifdef USE_SFML_RENDERWINDOW
	void setRenderTarget(sf::RenderWindow* target);
#endif
}

#endif//__DRAWING_H__
