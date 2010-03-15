
#include "Main.h"
#include "WxStuff.h"
#include "SplashWindow.h"
#include "ArchiveManager.h"
#include "Console.h"

SplashWindow* SplashWindow::instance = NULL;

SplashWindow::SplashWindow(wxWindow* parent)
: wxMiniFrame(parent, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP) {
	// Setup sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// Add logo image
	wxBitmap bm;
	string tempfile = appPath(_T("temp.png"), DIR_TEMP);
	ArchiveEntry* logo = theArchiveManager->programResourceArchive()->getEntry(_T("logo.png"));
	if (logo) {
		logo->exportFile(tempfile);
		bm.LoadFile(tempfile, wxBITMAP_TYPE_PNG);
	}

	sizer->Add(new wxStaticBitmap(this, -1, bm), 1, wxEXPAND);

	// Clean up
	wxRemoveFile(tempfile);
}

SplashWindow::~SplashWindow() {
}

void SplashWindow::showSplash(string message) {
	// Show & init window
	SetInitialSize(wxSize(300, 200));
	Layout();
	Centre();
	ShowWithEffect(wxSHOW_EFFECT_BLEND);
	Refresh();
}

void SplashWindow::hide() {
	HideWithEffect(wxSHOW_EFFECT_BLEND);
}


/* Console Command - "splash"
 * Shows the splash screen with the given message, or hides it if
 * no message is given
 *******************************************************************/
void c_sw_splash(vector<string> args) {
	if (args.size() == 0)
		theSplashWindow->hide();
	else
		theSplashWindow->showSplash(args[0]);
}
ConsoleCommand sw_splash(_T("splash"), &c_sw_splash, 0);
