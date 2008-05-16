
#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include "WadManagerPanel.h"
#include <wx/notebook.h>

class MainWindow : public wxFrame
{
private:
	WadManagerPanel*		panel_wadmanager;
	wxNotebook*				notebook_tabs;

public:
	MainWindow();
	~MainWindow();

	void	setupLayout();
	void	setMockLayout();
};

#endif //__MAINWINDOW_H__
