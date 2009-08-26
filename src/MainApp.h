
#ifndef __MAINAPP_H__
#define __MAINAPP_H__

#include <wx/log.h>

class SLADELog : public wxLog {
private:

public:
	SLADELog() {}
	~SLADELog() {}

	void DoLog(wxLogLevel level, const wxChar* str, time_t t);
};

class MainApp : public wxApp {
private:

public:
	virtual bool OnInit();
	virtual int OnExit();

	void	initLogFile();
	void	readConfigFile();
	void	saveConfigFile();
};

#endif //__MAINAPP_H__
