
#ifndef __MAINAPP_H__
#define __MAINAPP_H__

#include "MainWindow.h"
#include <wx/app.h>
#include <wx/log.h>

class SLADELog : public wxLog {
protected:
// wxWidgets thinks it's funny to have incompatible log interfaces between release numbers...
#if (wxMAJOR_VERSION < 2 || (wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9))
#error This will not compile with wxWidgets older than 2.9.0 !
#elif (wxMAJOR_VERSION > 2 || (wxMAJOR_VERSION == 2 && wxMINOR_VERSION >= 9 && wxRELEASE_NUMBER > 0))
	void DoLogText(const wxString& msg);
#endif

public:
	SLADELog() {}
	~SLADELog() {}

#if (wxMAJOR_VERSION == 2 && wxMINOR_VERSION == 9 && wxRELEASE_NUMBER == 0)
	void DoLogString(const wxString& msg, time_t t);
#endif
};

class MainApp : public wxApp {
private:
	MainWindow*		main_window;
	static MainApp*	instance;

public:
	virtual bool OnInit();
	virtual int OnExit();
	virtual void OnFatalException();

	MainWindow*	getMainWindow() { return main_window; }
	// Singleton implementation
	static MainApp*	getInstance() {
		if (!instance)
			wxLogMessage("This is fucked up");

		return instance;
	}
	static void setInstance(MainApp *app) { instance = app; }

	bool	initDirectories();
	void	initLogFile();
	void	readConfigFile();
	void	saveConfigFile();
};

// Define for less cumbersome MainWindow::getInstance()
#define theApp MainApp::getInstance()

#endif //__MAINAPP_H__
