
#ifndef __SPLASHWINDOW_H__
#define __SPLASHWINDOW_H__

#include <wx/minifram.h>

class SplashWindow : public wxMiniFrame {
private:

	static SplashWindow*	instance;

public:
	SplashWindow(wxWindow* parent);
	~SplashWindow();

	static SplashWindow* getInstance(wxWindow* parent = NULL) {
		if (!instance)
			instance = new SplashWindow(parent);

		return instance;
	}

	void	showSplash(string message);
	void	hide();
};

// Define for less cumbersome SplashWindow::getInstance()
#define theSplashWindow SplashWindow::getInstance()

#endif//__SPLASHWINDOW_H__
