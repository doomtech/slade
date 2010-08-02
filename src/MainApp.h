
#ifndef __MAINAPP_H__
#define __MAINAPP_H__

#include "MainWindow.h"
#include <wx/log.h>

class SLADELog : public wxLog {
protected:
	void DoLogText(const wxString& msg);

public:
	SLADELog() {}
	~SLADELog() {}

	//void DoLogString(const wxString& msg, time_t t);
};

class MainApp : public wxApp {
private:
	MainWindow*	main_window;

public:
	virtual bool OnInit();
	virtual int OnExit();
	virtual void OnFatalException();

	MainWindow*	getMainWindow() { return main_window; }

	bool	initDirectories();
	void	initLogFile();
	void	readConfigFile();
	void	saveConfigFile();
};

#endif //__MAINAPP_H__
