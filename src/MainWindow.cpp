
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
#include <wx/html/htmlwin.h>


/* MainWindow::MainWindow
 * MainWindow class constructor
 *******************************************************************/
MainWindow::MainWindow()
    : wxFrame((wxFrame *) NULL, -1, _T("SLADE"), wxPoint(0, 0), wxSize(800, 600))
{
	//setMockLayout();
	setupLayout();
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
	wxMenu* fileMenu = new wxMenu(_T(""));
    fileMenu->Append(-1, _("&Quit\tAlt-F4"), _("Quit SLADE"));
    menu->Append(fileMenu, _("&File"));

	// Set the menu
	SetMenuBar(menu);


	// -- Wad Manager Panel --
	panel_wadmanager = new WadManagerPanel(this);

	// Setup panel info & add panel
	p_inf.DefaultPane();
	p_inf.Left();
	p_inf.BottomDockable(false);
	p_inf.TopDockable(false);
	p_inf.BestSize(192, 480);
	p_inf.Caption(_T("Wad Manager"));
	m_mgr->AddPane(panel_wadmanager, p_inf);


	// -- Editor Area --
	notebook_tabs = new wxNotebook(this, -1, wxDefaultPosition, wxDefaultSize, wxNB_TOP);

	// Setup panel info & add panel
	p_inf.CenterPane();
	m_mgr->AddPane(notebook_tabs, p_inf);

	// Create Start Page (temporary)
	wxHtmlWindow *html_startpage = new wxHtmlWindow(notebook_tabs, -1);
	notebook_tabs->AddPage(html_startpage, _T("Start Page"));
	html_startpage->SetPage(_T("<HTML><BODY><CENTER><H1>SLADE</H1><BR>It's A Doom Editor<BR><BR><BR>(Stuff will go here eventually)</CENTER></BODY></HTML>"));


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
	wxAuiManager *m_mgr = new wxAuiManager(this);


	// Menu bar
	wxMenuBar *menu = new wxMenuBar();
	wxMenu* fileMenu = new wxMenu(_T(""));
    fileMenu->Append(-1, _("&Quit\tAlt-F4"), _("Quit SLADE"));
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
}
