
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    MainWindow.cpp
 * Description: MainWindow class, ie the main SLADE window
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "WxStuff.h"
#include "MainWindow.h"
#include <wx/aui/aui.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
string	aui_perspective = _T("");


/* MainWindow::MainWindow
 * MainWindow class constructor
 *******************************************************************/
MainWindow::MainWindow()
    : wxFrame((wxFrame *) NULL, -1, _T("SLADE"), wxPoint(0, 0), wxSize(800, 600))
{
	setupLayout();
	Maximize();
}

/* MainWindow::~MainWindow
 * MainWindow class destructor
 *******************************************************************/
MainWindow::~MainWindow()
{
}

/* MainWindow::setupLayout
 * Sets up the wxWidgets window layout
 *******************************************************************/
void MainWindow::setupLayout()
{
	// Create the wxAUI manager & related things
	wxAuiManager *m_mgr = new wxAuiManager(this);
	wxAuiPaneInfo p_inf;


	// -- Menu bar --
	wxMenuBar *menu = new wxMenuBar();

	// File menu
	wxMenu* fileMenu = new wxMenu(_(""));
    fileMenu->Append(MENU_FILE_OPEN,	_("&Open Wad"),		_("Open a Wad file"));
    fileMenu->Append(MENU_FILE_QUIT,	_("&Quit"),			_("Quit SLADE"));
    menu->Append(fileMenu, _("&File"));

	// Edit menu
	wxMenu* editMenu = new wxMenu(_T(""));
	menu->Append(editMenu, _("&Edit"));

	// View menu
	wxMenu* viewMenu = new wxMenu(_T(""));
	viewMenu->Append(MENU_VIEW_WADMANAGER,	_("&Wad Manager"),	_T("Show the wad manager"));
	menu->Append(viewMenu, _("&View"));

	// Set the menu
	SetMenuBar(menu);


	// -- Editor Area --
	notebook_tabs = new wxAuiNotebook(this, -1, wxDefaultPosition, wxDefaultSize);

	// Setup panel info & add panel
	p_inf.CenterPane();
	m_mgr->AddPane(notebook_tabs, p_inf);

	// Create Start Page (temporary)
	html_startpage = new wxHtmlWindow(notebook_tabs, HTML_WINDOW);
	notebook_tabs->AddPage(html_startpage, _("Start Page"));
	//html_startpage->SetPage(_("<HTML><BODY><CENTER><H1>SLADE<FONT SIZE=-4>3</FONT></H1><BR>It's A Doom Editor<BR><BR><BR>(Stuff will go here eventually)</CENTER></BODY></HTML>"));
	html_startpage->SetPage(_("<HTML><BODY><CENTER><H1>SLADE<FONT SIZE=-4>3</FONT></H1><BR>It's A Doom Editor<BR><BR><BR><A HREF=http://slade.mancubus.net>http://slade.mancubus.net</A></CENTER></BODY></HTML>"));


	// -- Wad Manager Panel --
	panel_wadmanager = new WadManagerPanel(this, notebook_tabs);

	// Setup panel info & add panel
	p_inf.DefaultPane();
	p_inf.Left();
	p_inf.BottomDockable(false);
	p_inf.TopDockable(false);
	p_inf.BestSize(192, 480);
	p_inf.Caption(_("Wad Manager"));
	m_mgr->AddPane(panel_wadmanager, p_inf);


	// -- Status Bar --
	CreateStatusBar();


	// Finalize
	m_mgr->Update();
	Layout();
}

/* MainWindow::setMockLayout
 * wxAUI testing stuff
 *******************************************************************/
void MainWindow::setMockLayout()
{
	/*
	wxAuiManager *m_mgr = new wxAuiManager(this);


	// Menu bar
	wxMenuBar *menu = new wxMenuBar();

	// File menu
	wxMenu* fileMenu = new wxMenu(_T(""));
	fileMenu->Append(MENU_FILE_OPEN,	_("&Open Wad"),		_("Opens a Wad file"));
    fileMenu->Append(MENU_FILE_QUIT,	_("&Quit"),			_("Quit SLADE"));
    menu->Append(fileMenu, _("&File"));
	SetMenuBar(menu);


	// Toolbars
	wxAuiPaneInfo p_inf;
	p_inf.ToolbarPane();
	p_inf.Top();

	wxToolBar *tb1 = new wxToolBar(this, -1);
	tb1->AddTool(-1, _("1"), wxBitmap(16, 16));
	tb1->AddTool(-1, _("2"), wxBitmap(16, 16));
	tb1->AddTool(-1, _("3"), wxBitmap(16, 16));
	tb1->Realize();
	m_mgr->AddPane(tb1, p_inf);

	p_inf.Position(1);

	wxToolBar *tb2 = new wxToolBar(this, -1);
	tb2->AddTool(-1, _("1"), wxBitmap(16, 16));
	tb2->AddTool(-1, _("2"), wxBitmap(16, 16));
	tb2->AddTool(-1, _("3"), wxBitmap(16, 16));
	tb2->Realize();
	m_mgr->AddPane(tb2, p_inf);


	// Map canvas
	p_inf.CenterPane();
	p_inf.Row(0);

	wxPanel *canvas = new wxPanel(this, -1);
	m_mgr->AddPane(canvas, p_inf);


	// Info Bar
	p_inf.Bottom();
	p_inf.DefaultPane();
	p_inf.BestSize(160, 160);

	wxPanel *infobar = new wxPanel(this, -1);
	m_mgr->AddPane(infobar, p_inf);

	m_mgr->Update();
	*/
}

/*******************************************************************
 * WXWIDGETS EVENTS & HANDLERS
 *******************************************************************/
BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	// MENU

	// File Menu
	//EVT_MENU(MENU_FILE_NEW, MainWindow::onFileNew)
	//EVT_MENU(MENU_FILE_NEWZIP, MainWindow::onFileNewZip)
	EVT_MENU(MENU_FILE_OPEN, MainWindow::onFileOpen)
	//EVT_MENU(MENU_FILE_SAVEALL, MainWindow::onFileSaveAll)
	//EVT_MENU(MENU_FILE_CLOSEALL, MainWindow::onFileCloseAll)
	//EVT_MENU(MENU_FILE_OPTIONS, MainWindow::onFileOptions)
	EVT_MENU(MENU_FILE_QUIT, MainWindow::onFileQuit)
	//EVT_MENU_RANGE(MENU_FILE_RECENT, MENU_FILE_RECENT + 200, MainWindow::onFileRecent)

	// Edit Menu

	// View Menu
	EVT_MENU(MENU_VIEW_WADMANAGER, MainWindow::onViewWadManager)

	EVT_HTML_LINK_CLICKED(HTML_WINDOW, MainWindow::onHTMLLinkClicked)
END_EVENT_TABLE()

/* MainWindow::onFileOpen
 * File->Open menu item event handler.
 *******************************************************************/
void MainWindow::onFileOpen(wxCommandEvent &e)
{
	// Open a file browser dialog that allows multiple selection
	// and filters by wad, zip and pk3 file extensions
	wxFileDialog *dialog_open = new wxFileDialog(this, _T("Choose file(s) to open"), wxEmptyString, wxEmptyString, 
		_T("Any Supported File (*.wad; *.zip; *.pk3)|*.wad;*.zip;*.pk3|Doom Wad files (*.wad)|*.wad|Zip files (*.zip)|*.zip|Pk3 (zip) files (*.pk3)|*.pk3"),
		wxOPEN|wxMULTIPLE|wxFILE_MUST_EXIST, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_open->ShowModal() == wxID_OK)
	{
		// Get an array of selected filenames
		wxArrayString files;
		dialog_open->GetPaths(files);

		// Send it to the Wad Manager Panel
		panel_wadmanager->openFiles(files);
	}
}

/* MainWindow::onFileQuit
 * File->Quit menu item event handler.
 *******************************************************************/
void MainWindow::onFileQuit(wxCommandEvent &e)
{
	wxTheApp->ExitMainLoop();
}

/* MainWindow::onViewWadManager
 * View->Wad Manager menu item event handler.
 *******************************************************************/
void MainWindow::onViewWadManager(wxCommandEvent &e)
{
	wxAuiManager *m_mgr = wxAuiManager::GetManager(panel_wadmanager);
	m_mgr->GetPane(panel_wadmanager).Show(true);
	m_mgr->Update();
}

/* MainWindow::onHTMLLinkClicked
 * Called when a link is clicked on the HTML Window, so that
 * external (http) links are opened in the default browser
 *******************************************************************/
void MainWindow::onHTMLLinkClicked(wxHtmlLinkEvent &e)
{
	if (e.GetLinkInfo().GetHref().StartsWith(_T("http://")))
		wxLaunchDefaultBrowser(e.GetLinkInfo().GetHref());
	else
		html_startpage->OnLinkClicked(e.GetLinkInfo());
}
