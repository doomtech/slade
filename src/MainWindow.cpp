
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
#include "ConsolePanel.h"
#include "ArchiveManager.h"
#include "Archive.h"
#include "Icons.h"
#include "BaseResourceArchivesPanel.h"
#include <wx/aboutdlg.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
string main_window_layout = "";


/*******************************************************************
 * MAINWINDOW CLASS FUNCTIONS
 *******************************************************************/

/* MainWindow::MainWindow
 * MainWindow class constructor
 *******************************************************************/
MainWindow::MainWindow()
: wxFrame((wxFrame *) NULL, -1, "SLADE", wxPoint(0, 0), wxSize(1024, 768)) {
	Maximize();
	setupLayout();
}

/* MainWindow::~MainWindow
 * MainWindow class destructor
 *******************************************************************/
MainWindow::~MainWindow() {
}

/* MainWindow::setupLayout
 * Sets up the wxWidgets window layout
 *******************************************************************/
void MainWindow::setupLayout() {
	// Create the wxAUI manager & related things
	m_mgr = new wxAuiManager(this);
	wxAuiPaneInfo p_inf;

	// Set icon
	string icon_filename = appPath("slade.ico", DIR_TEMP);
	theArchiveManager->programResourceArchive()->getEntry("slade.ico")->exportFile(icon_filename);
	SetIcon(wxIcon(icon_filename, wxBITMAP_TYPE_ICO));
	wxRemoveFile(icon_filename);


	// -- Menu bar --
	wxMenuBar *menu = new wxMenuBar();

	// File->New Submenu
	wxMenu* fileNewMenu = new wxMenu("");
	fileNewMenu->Append(createMenuItem(fileNewMenu, MENU_FILE_NEW,		"&Wad Archive\tCtrl+Shift+W",	"Create a new Doom Wad Archive",			"t_newarchive"));
	fileNewMenu->Append(createMenuItem(fileNewMenu, MENU_FILE_NEWZIP,	"&Zip Archive\tCtrl+Shift+Z",	"Create a new Zip Archive (zip/pk3/jdf)",	"t_newzip"));

	// File menu
	wxMenu* fileMenu = new wxMenu(_(""));
	fileMenu->AppendSubMenu(fileNewMenu,	"&New",				"Create a new Archive");
	fileMenu->AppendSeparator();
    fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_OPEN,		"&Open\tCtrl+O",		"Open an existing Archive",							"t_open"));
	fileMenu->AppendSeparator();
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_SAVE,		"&Save\tCtrl+S",		"Save the currently open Archive",					"t_save"));
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_SAVEAS,		"Save &As...",			"Save the currently open Archive to a new file",	"t_saveas"));
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_SAVEALL,	"Save All",				"Save all open Archives",							"t_saveall"));
	fileMenu->AppendSeparator();
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_CLOSE,		"&Close",				"Close the currently open Archive",					"t_close"));
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_CLOSEALL,	"Close All",			"Close all open Archives",							"t_closeall"));
	fileMenu->AppendSeparator();
    fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_QUIT,		"&Quit",				"Quit SLADE"));
	menu->Append(fileMenu, "&File");

	// Editor menu
	wxMenu* editorMenu = new wxMenu("");
	editorMenu->Append(createMenuItem(editorMenu, MENU_EDITOR_SET_BASE_RESOURCE,	"Set &Base Resource Archive",	"Set the base resource archive, to act as the 'program IWAD'"));
	editorMenu->Append(createMenuItem(editorMenu, MENU_EDITOR_PREFERENCES,			"&Preferences...",				"Setup SLADE options and preferences"));
	menu->Append(editorMenu, "E&ditor");

	// Entry menu
	wxMenu* entryMenu = new wxMenu("");
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_NEW,			"New",				"Create a new empty entry",								"t_newentry"));
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_NEWFROMFILE,	"New from File",	"Create a new entry with data from a file"));
	entryMenu->AppendSeparator();
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_RENAME,		"Rename",			"Rename the selected entries",							"t_rename"));
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_DELETE,		"Delete",			"Delete the selected entries",							"t_delete"));
	entryMenu->AppendSeparator();
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_IMPORT,		"Import",			"Import a file to the selected entry",					"t_import"));
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_EXPORT,		"Export",			"Export the selected entries to files",					"t_export"));
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_EXPORTWAD,	"Export as Wad",	"Export the selected entries to a new Wad Archive"));
	entryMenu->AppendSeparator();
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_MOVEUP,		"Move Up",			"Move the selected entries up",							"t_up"));
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_MOVEDOWN,	"Move Down",		"Move the selected entries down",						"t_down"));
	menu->Append(entryMenu, "&Entry");

	// View menu
	wxMenu* viewMenu = new wxMenu("");
	viewMenu->Append(MENU_VIEW_MANAGER,	"&Archive Manager\tCtrl+1",	"Toggle the archive manager");
	viewMenu->Append(MENU_VIEW_CONSOLE,	"&Console\tCtrl+2",			"Toggle the console");
	menu->Append(viewMenu, "&View");

	// Help menu
	wxMenu* helpMenu = new wxMenu("");
	helpMenu->Append(MENU_HELP_ABOUT,	"&About",	"Information about SLADE");
	menu->Append(helpMenu, "&Help");

	// Set the menu
	SetMenuBar(menu);



	// -- Toolbars --

	// Create File toolbar
	wxAuiToolBar* tb_file = new wxAuiToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	tb_file->AddTool(MENU_FILE_NEW, "New Wad", getIcon("t_newarchive"), "New Wad Archive");
	tb_file->AddTool(MENU_FILE_NEWZIP, "New Zip", getIcon("t_newzip"), "New Zip Archive");
	tb_file->AddTool(MENU_FILE_OPEN, "Open", getIcon("t_open"), "Open");
	tb_file->AddTool(MENU_FILE_SAVE, "Save", getIcon("t_save"), "Save");
	tb_file->AddTool(MENU_FILE_SAVEAS, "Save As", getIcon("t_saveas"), "Save As");
	tb_file->AddTool(MENU_FILE_SAVEALL, "Save All", getIcon("t_saveall"), "Save All");
	tb_file->AddTool(MENU_FILE_CLOSE, "Close", getIcon("t_close"), "Close");
	tb_file->AddTool(MENU_FILE_CLOSEALL, "Close All", getIcon("t_closeall"), "Close All");
	tb_file->Realize();

	// Create Entry toolbar
	wxAuiToolBar* tb_entry = new wxAuiToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	tb_entry->AddTool(MENU_ENTRY_NEW, 		"New",			getIcon("t_newentry"),	"New Entry");
	tb_entry->AddTool(MENU_ENTRY_RENAME,	"Rename",		getIcon("t_rename"),	"Rename");
	tb_entry->AddTool(MENU_ENTRY_DELETE,	"Delete",		getIcon("t_delete"),	"Delete");
	tb_entry->AddTool(MENU_ENTRY_IMPORT,	"Import",		getIcon("t_import"),	"Import");
	tb_entry->AddTool(MENU_ENTRY_EXPORT,	"Export",		getIcon("t_export"),	"Export");
	tb_entry->AddTool(MENU_ENTRY_MOVEUP,	"Move Up",		getIcon("t_up"),		"Move Up");
	tb_entry->AddTool(MENU_ENTRY_MOVEDOWN,	"Move Down",	getIcon("t_down"),		"Move Down");
	tb_entry->Realize();

	// Setup panel info & add toolbar panels
	// File toolbar
	p_inf.ToolbarPane();
	p_inf.Top();
	p_inf.Name("tb_file");
	m_mgr->AddPane(tb_file, p_inf);

	// Entry toolbar
	p_inf.ToolbarPane();
	p_inf.Top();
	p_inf.Position(1);
	p_inf.Name("tb_entry");
	m_mgr->AddPane(tb_entry, p_inf);



	// -- Editor Area --
	notebook_tabs = new wxAuiNotebook(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE|wxNO_BORDER|wxAUI_NB_WINDOWLIST_BUTTON);

	// Setup panel info & add panel
	p_inf.CenterPane();
	p_inf.Name("editor_area");
	p_inf.PaneBorder(false);
	m_mgr->AddPane(notebook_tabs, p_inf);

	// Create Start Page (temporary)
	html_startpage = new wxHtmlWindow(notebook_tabs, -1);
	html_startpage->SetName("startpage");
	notebook_tabs->AddPage(html_startpage,"Start Page");
	Archive* res_archive = theArchiveManager->programResourceArchive();
	ArchiveEntry* sp_entry = res_archive->getEntry("html/startpage.htm");
	if (sp_entry)
		html_startpage->SetPage(wxString::From8BitData((const char *)(sp_entry->getData(true)), sp_entry->getSize()));
	else { // Fallback
		wxLogMessage("Warning: html/startpage.htm not found in slade.pk3!");
		html_startpage->SetPage("<HTML><BODY><CENTER><H1>SLADE<FONT SIZE=-4>3</FONT></H1><BR>It's A Doom Editor<BR><BR><BR><A HREF=http://slade.mancubus.net>http://slade.mancubus.net</A></CENTER></BODY></HTML>");
	}


	// -- Archive Manager Panel --
	panel_archivemanager = new ArchiveManagerPanel(this, notebook_tabs);

	// Setup panel info & add panel
	p_inf.DefaultPane();
	p_inf.Left();
	p_inf.BottomDockable(false);
	p_inf.TopDockable(false);
	p_inf.BestSize(192, 480);
	p_inf.Caption("Archive Manager");
	p_inf.Name("archive_manager");
	m_mgr->AddPane(panel_archivemanager, p_inf);



	// -- Console Panel --
	ConsolePanel *panel_console = new ConsolePanel(this, -1);

	// Setup panel info & add panel
	p_inf.DefaultPane();
	p_inf.Float();
	p_inf.FloatingSize(600, 400);
	p_inf.FloatingPosition(100, 100);
	p_inf.Show(false);
	p_inf.Caption("Console");
	p_inf.Name("console");
	m_mgr->AddPane(panel_console, p_inf);


	// -- Status Bar --
	CreateStatusBar();


	// Load previously saved perspective string
	long vers = 0;
	main_window_layout.Left(3).ToLong(&vers);
	if (vers == MW_LAYOUT_VERS)
		m_mgr->LoadPerspective(main_window_layout.Right(main_window_layout.Length() - 3));

	// Finalize
	m_mgr->Update();
	Layout();

	// Bind events
	html_startpage->Bind(wxEVT_COMMAND_HTML_LINK_CLICKED, &MainWindow::onHTMLLinkClicked, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::onMenuItemClicked, this, MENU_START, MENU_END);
	Bind(wxEVT_CLOSE_WINDOW, &MainWindow::onClose, this);
}


/*******************************************************************
 * MAINWINDOW EVENTS
 *******************************************************************/

/* MainWindow::onMenuItemClicked
 * Called when a menu or toolbar item is clicked
 *******************************************************************/
void MainWindow::onMenuItemClicked(wxCommandEvent& e) {
	// *******************************************************
	// FILE MENU
	// *******************************************************

	// File->New->Wad Archive
	if (e.GetId() == MENU_FILE_NEW)
		panel_archivemanager->createNewArchive(ARCHIVE_WAD);

	// File->New->Zip Archive
	else if (e.GetId() == MENU_FILE_NEWZIP)
		panel_archivemanager->createNewArchive(ARCHIVE_ZIP);

	// File->Open
	else if (e.GetId() == MENU_FILE_OPEN) {
		// Create extensions string
		string extensions = theArchiveManager->getArchiveExtensionsString();

		// Open a file browser dialog that allows multiple selection
		// and filters by wad, zip and pk3 file extensions
		wxFileDialog *dialog_open = new wxFileDialog(this, "Choose file(s) to open", wxEmptyString, wxEmptyString, extensions, wxFD_OPEN|wxFD_MULTIPLE|wxFD_FILE_MUST_EXIST, wxDefaultPosition);

		// Run the dialog & check that the user didn't cancel
		if (dialog_open->ShowModal() == wxID_OK) {
			wxBeginBusyCursor();

			// Get an array of selected filenames
			wxArrayString files;
			dialog_open->GetPaths(files);

			// Send it to the Archive Manager Panel
			panel_archivemanager->openFiles(files);

			wxEndBusyCursor();
		}
	}

	// File->Save
	else if (e.GetId() == MENU_FILE_SAVE) {
		wxBeginBusyCursor();
		panel_archivemanager->saveCurrent();
		wxEndBusyCursor();
	}

	// File->Save As
	else if (e.GetId() == MENU_FILE_SAVEAS) {
		wxBeginBusyCursor();
		panel_archivemanager->saveCurrentAs();
		wxEndBusyCursor();
	}

	// File->Save All
	else if (e.GetId() == MENU_FILE_SAVEALL) {
		wxBeginBusyCursor();
		panel_archivemanager->saveAll();
		wxEndBusyCursor();
	}

	// File->Close
	else if (e.GetId() == MENU_FILE_CLOSE)
		panel_archivemanager->closeCurrent();

	// File->Close All
	else if (e.GetId() == MENU_FILE_CLOSEALL)
		panel_archivemanager->closeAll();

	// File->Quit
	else if (e.GetId() == MENU_FILE_QUIT)
		this->Close(true);
		//wxTheApp->ExitMainLoop();


	// *******************************************************
	// EDITOR MENU
	// *******************************************************

	// Editor->Set Base Resource Archive
	else if (e.GetId() == MENU_EDITOR_SET_BASE_RESOURCE) {
		wxDialog dialog_ebr(this, -1, "Edit Base Resource Archives", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);
		BaseResourceArchivesPanel brap(&dialog_ebr);

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(&brap, 1, wxEXPAND|wxALL, 4);

		sizer->Add(dialog_ebr.CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxDOWN, 4);

		dialog_ebr.SetSizer(sizer);
		dialog_ebr.Layout();
		dialog_ebr.SetInitialSize(wxSize(400, 240));
		if (dialog_ebr.ShowModal() == wxID_OK)
			theArchiveManager->openBaseResource(brap.getSelectedPath());
	}

	// *******************************************************
	// ENTRY MENU
	// *******************************************************

	// Entry->New
	else if (e.GetId() == MENU_ENTRY_NEW)
		panel_archivemanager->newEntry();

	// Entry->New From File
	else if (e.GetId() == MENU_ENTRY_NEWFROMFILE)
		panel_archivemanager->newEntryFromFile();

	// Entry->Rename
	else if (e.GetId() == MENU_ENTRY_RENAME)
		panel_archivemanager->renameEntry();

	// Entry->Delete
	else if (e.GetId() == MENU_ENTRY_DELETE)
		panel_archivemanager->deleteEntry();

	// Entry->Import
	else if (e.GetId() == MENU_ENTRY_IMPORT) {
		wxBeginBusyCursor();
		panel_archivemanager->importEntry();
		wxEndBusyCursor();
	}

	// Entry->Export
	else if (e.GetId() == MENU_ENTRY_EXPORT) {
		wxBeginBusyCursor();
		panel_archivemanager->exportEntry();
		wxEndBusyCursor();
	}

	// Entry->Export as Wad
	else if (e.GetId() == MENU_ENTRY_EXPORTWAD) {
		wxBeginBusyCursor();
		panel_archivemanager->exportEntryWad();
		wxEndBusyCursor();
	}

	// Entry->Move Up
	else if (e.GetId() == MENU_ENTRY_MOVEUP)
		panel_archivemanager->moveUp();

	// Entry->Move Down
	else if (e.GetId() == MENU_ENTRY_MOVEDOWN)
		panel_archivemanager->moveDown();


	// *******************************************************
	// VIEW MENU
	// *******************************************************

	// View->Archive Manager
	else if (e.GetId() == MENU_VIEW_MANAGER) {
		wxAuiManager *m_mgr = wxAuiManager::GetManager(panel_archivemanager);
		wxAuiPaneInfo& p_inf = m_mgr->GetPane("archive_manager");
		p_inf.Show(!p_inf.IsShown());
		m_mgr->Update();
	}

	// View->Console
	else if (e.GetId() == MENU_VIEW_CONSOLE) {
		wxAuiManager *m_mgr = wxAuiManager::GetManager(panel_archivemanager);
		wxAuiPaneInfo& p_inf = m_mgr->GetPane("console");
		p_inf.Show(!p_inf.IsShown());
		m_mgr->Update();
	}

	// *******************************************************
	// HELP MENU
	// *******************************************************

	// Help->About
	else if (e.GetId() == MENU_HELP_ABOUT) {
		wxAboutDialogInfo info;
		info.SetName("SLADE");
		info.SetVersion(Global::version);
		info.SetWebSite("http://slade.mancubus.net");
		info.SetDescription("It's a Doom Editor");

		// Set icon
		string icon_filename = appPath("slade.ico", DIR_TEMP);
		theArchiveManager->programResourceArchive()->getEntry("slade.ico")->exportFile(icon_filename);
		info.SetIcon(wxIcon(icon_filename, wxBITMAP_TYPE_ICO));
		wxRemoveFile(icon_filename);

		string year = wxNow().Right(4);
		info.SetCopyright(s_fmt("(C) 2008-%s Simon Judd <sirjuddington@gmail.com>", year.c_str()));

		wxAboutBox(info);
	}
}

/* MainWindow::onHTMLLinkClicked
 * Called when a link is clicked on the HTML Window, so that
 * external (http) links are opened in the default browser
 *******************************************************************/
void MainWindow::onHTMLLinkClicked(wxHtmlLinkEvent &e) {
	if (e.GetLinkInfo().GetHref().StartsWith("http://"))
		wxLaunchDefaultBrowser(e.GetLinkInfo().GetHref());
	else
		html_startpage->OnLinkClicked(e.GetLinkInfo());
}

/* MainWindow::onClose
 * Called when the window is closed
 *******************************************************************/
void MainWindow::onClose(wxCloseEvent& e) {
	main_window_layout = m_mgr->SavePerspective();
	wxTheApp->Exit();
	e.Skip();
}
