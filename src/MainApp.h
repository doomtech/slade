
#ifndef __MAINAPP_H__
#define __MAINAPP_H__

#include <wx/log.h>

class SLADELog : public wxLog {
private:

public:
	SLADELog() {}
	~SLADELog() {}

	void DoLogString(const wxString& msg, time_t t);
};

class MainApp : public wxApp {
private:

public:
	virtual bool OnInit();
	virtual int OnExit();
	virtual void OnFatalException();

	bool	initDirectories();
	void	initLogFile();
	void	readConfigFile();
	void	saveConfigFile();
};

#endif //__MAINAPP_H__
