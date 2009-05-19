
#ifndef __MAINAPP_H__
#define __MAINAPP_H__

class MainApp : public wxApp {
private:

public:
	virtual bool OnInit();
	virtual int OnExit();

	void initLogFile();
};

#endif //__MAINAPP_H__
