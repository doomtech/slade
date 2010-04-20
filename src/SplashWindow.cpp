
#include "Main.h"
#include "WxStuff.h"
#include "SplashWindow.h"
#include "ArchiveManager.h"
#include "Console.h"
#include <wx/dcbuffer.h>

SplashWindow*	SplashWindow::instance = NULL;
wxBitmap		SplashWindow::bm_logo;
int				SplashWindow::width = 300;
int				SplashWindow::height = 204;

SplashWindow::SplashWindow()
: wxMiniFrame(NULL, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP|wxBORDER_NONE) {
	// Init
	show_progress = false;
	progress = 0.0f;
	progress_indefinite_anim = 0.0f;
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	SetBackgroundColour(wxColour(180, 186, 200));

	// Bind events
	Bind(wxEVT_PAINT, &SplashWindow::onPaint, this);

	// Hide initially
	Show(false);
}

SplashWindow::~SplashWindow() {
}

void SplashWindow::setMessage(string message) {
	this->message = message;
	forceRedraw();
}

void SplashWindow::setProgressMessage(string message) {
	message_progress = message;
	forceRedraw();
}

void SplashWindow::setProgress(float progress) {
	this->progress = progress;

	// Refresh if last redraw was > 20ms ago
	if (timer.Time() >= 20)
		forceRedraw();
}

void SplashWindow::init() {
	// Load logo image
	string tempfile = appPath(_T("temp.png"), DIR_TEMP);
	ArchiveEntry* logo = theArchiveManager->programResourceArchive()->getEntry(_T("logo.png"));
	if (logo) {
		logo->exportFile(tempfile);
		bm_logo.LoadFile(tempfile, wxBITMAP_TYPE_PNG);
	}

	// Clean up
	wxRemoveFile(tempfile);
}

void SplashWindow::show(string message, bool progress) {
	// Setup progress bar
	int rheight = height;
	if (progress) {
		show_progress = true;
		setProgress(0.0f);
		rheight += 10;
	}
	else
		show_progress = false;

	// Show & init window
	SetInitialSize(wxSize(width, rheight));
	Centre();
	setMessage(message);
	Show();
	forceRedraw();
}

void SplashWindow::hide() {
	// Destroy instance
	deleteInstance();
}

void SplashWindow::forceRedraw() {
	Refresh();
	Update();
}


void SplashWindow::onPaint(wxPaintEvent& e) {
	// Create device context
	wxAutoBufferedPaintDC dc(this);

	// Draw border
	dc.SetBrush(wxBrush(wxColour(180, 186, 200)));
	dc.SetPen(wxPen(wxColour(100, 104, 116)));
	dc.DrawRectangle(0, 0, width, height);

	// Draw SLADE logo
	dc.DrawBitmap(bm_logo, 0, 0, true);

	// Setup text
	wxFont font(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, _T("Calibri"));
	dc.SetFont(font);

	// Draw message
	wxSize text_size = dc.GetTextExtent(message);
	wxCoord x = (width*0.5) - int((double)text_size.GetWidth() * 0.5);
	wxCoord y = (height-4) - text_size.GetHeight();
	dc.DrawText(message, x, y);

	// Draw progress bar if necessary
	if (show_progress) {
		// Setup progress bar
		wxRect rect_pbar(0, height-4, width, 14);

		// Draw background
		dc.SetBrush(wxBrush(wxColour(40, 40, 56)));
		dc.DrawRectangle(rect_pbar);

		// Draw meter
		if (progress >= 0) {
			rect_pbar.SetRight(progress * width);
			dc.SetBrush(wxBrush(wxColour(100, 120, 255)));
			dc.DrawRectangle(rect_pbar);
		}
		else {
			// Negative progress means indefinite, show animation

			// Determine gradient colours
			float interval = progress_indefinite_anim * 2;
			if (interval > 1.0f)
				interval = 2.0f - interval;

			wxColor left(100 - (60.0f*interval), 120 - (80.0f*interval), 255 - (199.0f*interval));
			wxColor right(40 + (60.0f*interval), 40 + (80.0f*interval), 56 + (199.0f*interval));

			// Draw the animated meter
			rect_pbar.Deflate(1, 1);
			dc.GradientFillLinear(rect_pbar, left, right);

			// Increase animation counter
			progress_indefinite_anim += 0.02f;
			if (progress_indefinite_anim > 1.0f)
				progress_indefinite_anim = 0.0f;
		}

		// Draw text
		font.SetPointSize(8);
		dc.SetFont(font);
		text_size = dc.GetTextExtent(message_progress);
		x = (width*0.5) - int((double)text_size.GetWidth() * 0.5);
		y = height-4;
		dc.SetTextForeground(wxColour(200, 210, 255));
		dc.DrawText(message_progress, x, y);
	}

	timer.Start();
}

/*
void SplashWindow::onTimerNotify(wxTimerEvent& e) {
	Refresh();
	Update();
}
*/


/* Console Command - "splash"
 * Shows the splash screen with the given message, or hides it if
 * no message is given
 *******************************************************************/
void c_sw_splash(vector<string> args) {
	if (args.size() == 0)
		theSplashWindow->hide();
	else if (args.size() == 1)
		theSplashWindow->show(args[0]);
	else {
		theSplashWindow->show(args[0], true);
		float prog = atof(chr(args[1]));
		theSplashWindow->setProgress(prog);
	}
}
ConsoleCommand sw_splash(_T("splash"), &c_sw_splash, 0);
