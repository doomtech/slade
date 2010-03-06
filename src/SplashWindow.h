
#ifndef __SPLASHWINDOW_H__
#define __SPLASHWINDOW_H__

#include "GfxCanvas.h"
#include <wx/minifram.h>

class SplashCanvas : public OGLCanvas {
private:
	
public:
	SplashCanvas(wxWindow* parent);
	~SplashCanvas();
	
	void draw();
};

class SplashWindow : public wxMiniFrame {
private:
	SplashCanvas*	gl_splash;

	static SplashWindow*	instance;

public:
	SplashWindow();
	~SplashWindow();

	static SplashWindow* getInstance() {
		if (!instance)
			instance = new SplashWindow();

		return instance;
	}

	void	showSplash(string message);
};

// Define for less cumbersome SplashWindow::getInstance()
#define theSplashWindow SplashWindow::getInstance()

#endif//__SPLASHWINDOW_H__
