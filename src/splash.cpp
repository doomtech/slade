// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004-06               >>
// << ------------------------------------ >>
// << splash.cpp - Splash screen stuff     >>
// << ------------------------------------ >>

// Includes ------------------------------ >>
#include "main.h"
#include "wx_stuff.h"
#include "image_box.h"


class SplashWindow : public wxFrame
{
private:
	ImageBox		*logo;
	wxStaticText	*message;
	wxGauge			*progress;

public:
	SplashWindow();
	~SplashWindow();

	void SetProgress(int val) { progress->SetValue(val); }
	void SetMessage(string message);
	void ShowProgress(bool prog) { progress->Show(prog); }
};

SplashWindow *splash_window;

wxImage get_image_from_pk3(string entry_name, int type);

SplashWindow::SplashWindow()
//:	wxPanel(NULL, -1)
:	wxFrame(NULL, -1, _T(""), wxDefaultPosition, wxSize(590, -1), wxRAISED_BORDER)//wxCAPTION)
{
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	logo = new ImageBox(this, get_image_from_pk3("res/edit_tex/logo.png", wxBITMAP_TYPE_PNG));
	vbox->Add(logo, 0, wxEXPAND);

	message = new wxStaticText(this, -1, _T("Hello"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	vbox->Add(message, 0, wxEXPAND);

	progress = new wxGauge(this, -1, 100);
	vbox->Add(progress, 0, wxEXPAND);

	Layout();
	Show(true);
	SetBestFittingSize();
	Center();
}

SplashWindow::~SplashWindow()
{
}

void SplashWindow::SetMessage(string message)
{
	this->message->SetLabel(str_to_wx(message));
	Layout();
}

void setup_splash()
{
	splash_window = new SplashWindow();
}

void splash(string message, bool progbar)
{
	splash_window->Show();
	splash_window->ShowProgress(progbar);
	splash_window->SetMessage(message);
	splash_window->SetProgress(0);
	splash_window->SetBestFittingSize();
	splash_window->Layout();
	splash_window->Update();
}

void splash_hide()
{
	splash_window->Hide();
}

void splash_progress(double prog)
{
	splash_window->SetProgress(prog * 100);
	splash_window->Update();
}
