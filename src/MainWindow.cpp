
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
#include <wx/aboutdlg.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
string main_window_layout = _T("");


/*******************************************************************
 * MAINWINDOW CLASS FUNCTIONS
 *******************************************************************/

/* MainWindow::MainWindow
 * MainWindow class constructor
 *******************************************************************/
MainWindow::MainWindow()
: wxFrame((wxFrame *) NULL, -1, _T("SLADE"), wxPoint(0, 0), wxSize(800, 600)) {
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
	string icon_filename = appPath(_T("slade.ico"), DIR_TEMP);
	theArchiveManager->resourceArchive()->getEntry(_T("slade.ico"))->exportFile(icon_filename);
	SetIcon(wxIcon(icon_filename, wxBITMAP_TYPE_ICO));
	wxRemoveFile(icon_filename);


	// -- Menu bar --
	wxMenuBar *menu = new wxMenuBar();

	// File->New Submenu
	wxMenu* fileNewMenu = new wxMenu(_T(""));
	fileNewMenu->Append(createMenuItem(fileNewMenu, MENU_FILE_NEW,		_T("&Wad Archive\tCtrl+Shift+W"),	_T("Create a new Doom Wad Archive"),			_T("t_newarchive")));
	fileNewMenu->Append(createMenuItem(fileNewMenu, MENU_FILE_NEWZIP,	_T("&Zip Archive\tCtrl+Shift+Z"),	_T("Create a new Zip Archive (zip/pk3/jdf)"),	_T("t_newzip")));

	// File menu
	wxMenu* fileMenu = new wxMenu(_(""));
	fileMenu->AppendSubMenu(fileNewMenu,	_T("&New"),				_T("Create a new Archive"));
	fileMenu->AppendSeparator();
    fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_OPEN,		_T("&Open\tCtrl+O"),		_T("Open an existing Archive"),							_T("t_open")));
	fileMenu->AppendSeparator();
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_SAVE,		_T("&Save\tCtrl+S"),		_T("Save the currently open Archive"),					_T("t_save")));
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_SAVEAS,		_T("Save &As..."),			_T("Save the currently open Archive to a new file"),	_T("t_saveas")));
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_SAVEALL,	_T("Save All"),				_T("Save all open Archives"),							_T("t_saveall")));
	fileMenu->AppendSeparator();
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_CLOSE,		_T("&Close"),				_T("Close the currently open Archive"),					_T("t_close")));
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_CLOSEALL,	_T("Close All"),			_T("Close all open Archives"),							_T("t_closeall")));
	fileMenu->AppendSeparator();
    fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_QUIT,		_T("&Quit"),				_T("Quit SLADE")));
	menu->Append(fileMenu, _T("&File"));

	// Entry menu
	wxMenu* entryMenu = new wxMenu(_T(""));
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_NEW,			_T("New"),				_T("Create a new empty entry"),								_T("t_newentry")));
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_NEWFROMFILE,	_T("New from File"),	_T("Create a new entry with data from a file")));
	entryMenu->AppendSeparator();
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_RENAME,		_T("Rename"),			_T("Rename the selected entries"),							_T("t_rename")));
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_DELETE,		_T("Delete"),			_T("Delete the selected entries"),							_T("t_delete")));
	entryMenu->AppendSeparator();
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_IMPORT,		_T("Import"),			_T("Import a file to the selected entry"),					_T("t_import")));
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_EXPORT,		_T("Export"),			_T("Export the selected entries to files"),					_T("t_export")));
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_EXPORTWAD,	_T("Export as Wad"),	_T("Export the selected entries to a new Wad Archive")));
	entryMenu->AppendSeparator();
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_MOVEUP,		_T("Move Up"),			_T("Move the selected entries up"),							_T("t_up")));
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_MOVEDOWN,	_T("Move Down"),		_T("Move the selected entries down"),						_T("t_down")));
	menu->Append(entryMenu, _T("&Entry"));

	// View menu
	wxMenu* viewMenu = new wxMenu(_T(""));
	viewMenu->Append(MENU_VIEW_MANAGER,		_T("&Archive Manager\tCtrl+1"),	_T("Toggle the archive manager"));
	viewMenu->Append(MENU_VIEW_CONSOLE,		_T("&Console\tCtrl+2"),			_T("Toggle the console"));
	menu->Append(viewMenu, _T("&View"));

	// Help menu
	wxMenu* helpMenu = new wxMenu(_T(""));
	helpMenu->Append(MENU_HELP_ABOUT,	_T("&About"),	_T("Information about SLADE"));
	menu->Append(helpMenu, _T("&Help"));

	// Set the menu
	SetMenuBar(menu);



	// -- Toolbars --

	// Create File toolbar
	wxAuiToolBar* tb_file = new wxAuiToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	tb_file->AddTool(MENU_FILE_NEW, _T("New Wad"), getIcon(_T("t_newarchive")), _T("New Wad Archive"));
	tb_file->AddTool(MENU_FILE_NEWZIP, _T("New Zip"), getIcon(_T("t_newzip")), _T("New Zip Archive"));
	tb_file->AddTool(MENU_FILE_OPEN, _T("Open"), getIcon(_T("t_open")), _T("Open"));
	tb_file->AddTool(MENU_FILE_SAVE, _T("Save"), getIcon(_T("t_save")), _T("Save"));
	tb_file->AddTool(MENU_FILE_SAVEAS, _T("Save As"), getIcon(_T("t_saveas")), _T("Save As"));
	tb_file->AddTool(MENU_FILE_SAVEALL, _T("Save All"), getIcon(_T("t_saveall")), _T("Save All"));
	tb_file->AddTool(MENU_FILE_CLOSE, _T("Close"), getIcon(_T("t_close")), _T("Close"));
	tb_file->AddTool(MENU_FILE_CLOSEALL, _T("Close All"), getIcon(_T("t_closeall")), _T("Close All"));
	tb_file->Realize();

	// Create Entry toolbar
	wxAuiToolBar* tb_entry = new wxAuiToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	tb_entry->AddTool(MENU_ENTRY_NEW, _T("New"), getIcon(_T("t_newentry")), _T("New Entry"));
	tb_entry->AddTool(MENU_ENTRY_RENAME, _T("Rename"), getIcon(_T("t_rename")), _T("Rename"));
	tb_entry->AddTool(MENU_ENTRY_DELETE, _T("Delete"), getIcon(_T("t_delete")), _T("Delete"));
	tb_entry->AddTool(MENU_ENTRY_IMPORT, _T("Import"), getIcon(_T("t_import")), _T("Import"));
	tb_entry->AddTool(MENU_ENTRY_EXPORT, _T("Export"), getIcon(_T("t_export")), _T("Export"));
	tb_entry->AddTool(MENU_ENTRY_MOVEUP, _T("Move Up"), getIcon(_T("t_up")), _T("Move Up"));
	tb_entry->AddTool(MENU_ENTRY_MOVEDOWN, _T("Move Down"), getIcon(_T("t_down")), _T("Move Down"));
	tb_entry->Realize();

	wxAuiToolBar* tb_resource = new wxAuiToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	string test[] = { _T("test1"), _T("test2"), _T("test3") };
	tb_resource->AddControl(new wxChoice(tb_resource, -1, wxDefaultPosition, wxDefaultSize, 3, test), _T("Base Resource"));
	tb_resource->Realize();

	// Setup panel info & add toolbar panels
	// File toolbar
	p_inf.ToolbarPane();
	p_inf.Top();
	p_inf.Name(_T("tb_file"));
	m_mgr->AddPane(tb_file, p_inf);

	// Entry toolbar
	p_inf.ToolbarPane();
	p_inf.Top();
	p_inf.Position(1);
	p_inf.Name(_T("tb_entry"));
	m_mgr->AddPane(tb_entry, p_inf);

	// Resource toolbar
	p_inf.ToolbarPane();
	p_inf.Top();
	p_inf.Position(2);
	p_inf.Name(_T("tb_resource"));
	m_mgr->AddPane(tb_resource, p_inf);


	// -- Editor Area --
	notebook_tabs = new wxAuiNotebook(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE|wxNO_BORDER|wxAUI_NB_WINDOWLIST_BUTTON);

	// Setup panel info & add panel
	p_inf.CenterPane();
	p_inf.Name(_T("editor_area"));
	p_inf.PaneBorder(false);
	m_mgr->AddPane(notebook_tabs, p_inf);

	// Create Start Page (temporary)
	html_startpage = new wxHtmlWindow(notebook_tabs, -1);
	notebook_tabs->AddPage(html_startpage, _("Start Page"));
	html_startpage->SetPage(_("<HTML><BODY><CENTER><H1>SLADE<FONT SIZE=-4>3</FONT></H1><BR>It's A Doom Editor<BR><BR><BR><A HREF=http://slade.mancubus.net>http://slade.mancubus.net</A></CENTER></BODY></HTML>"));


	// -- Archive Manager Panel --
	panel_archivemanager = new ArchiveManagerPanel(this, notebook_tabs);

	// Setup panel info & add panel
	p_inf.DefaultPane();
	p_inf.Left();
	p_inf.BottomDockable(false);
	p_inf.TopDockable(false);
	p_inf.BestSize(192, 480);
	p_inf.Caption(_("Archive Manager"));
	p_inf.Name(_T("archive_manager"));
	m_mgr->AddPane(panel_archivemanager, p_inf);



	// -- Console Panel --
	ConsolePanel *panel_console = new ConsolePanel(this, -1);

	// Setup panel info & add panel
	p_inf.DefaultPane();
	p_inf.Float();
	p_inf.FloatingSize(600, 400);
	p_inf.FloatingPosition(100, 100);
	p_inf.Show(false);
	p_inf.Caption(_("Console"));
	p_inf.Name(_T("console"));
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
	string ext_wad = _T("*.wad;*.WAD;*.Wad");
	string ext_zip = _T("*.zip;*.ZIP;*.Zip");
	string ext_pk3 = _T("*.pk3;*.PK3;*.Pk3");
	string ext_jdf = _T("*.jdf;*.JDF;*.Jdf");

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
		string extensions = s_fmt(_T("Any Supported File (*.wad; *.zip; *.pk3; *.jdf)|%s;%s;%s;%s"), ext_wad.c_str(), ext_zip.c_str(), ext_pk3.c_str(), ext_jdf.c_str());
		extensions += s_fmt(_T("|Doom Wad files (*.wad)|%s"), ext_wad.c_str());
		extensions += s_fmt(_T("|Zip files (*.zip)|%s"), ext_zip.c_str());
		extensions += s_fmt(_T("|Pk3 (zip) files (*.pk3)|%s"), ext_pk3.c_str());
		extensions += s_fmt(_T("|JDF (zip) files (*.jdf)|%s"), ext_jdf.c_str());

		// Open a file browser dialog that allows multiple selection
		// and filters by wad, zip and pk3 file extensions
		wxFileDialog *dialog_open = new wxFileDialog(this, _T("Choose file(s) to open"), wxEmptyString, wxEmptyString, extensions, wxFD_OPEN|wxFD_MULTIPLE|wxFD_FILE_MUST_EXIST, wxDefaultPosition);

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
		wxAuiPaneInfo& p_inf = m_mgr->GetPane(_T("archive_manager"));
		p_inf.Show(!p_inf.IsShown());
		m_mgr->Update();
	}

	// View->Console
	else if (e.GetId() == MENU_VIEW_CONSOLE) {
		wxAuiManager *m_mgr = wxAuiManager::GetManager(panel_archivemanager);
		wxAuiPaneInfo& p_inf = m_mgr->GetPane(_T("console"));
		p_inf.Show(!p_inf.IsShown());
		m_mgr->Update();
	}

	// *******************************************************
	// HELP MENU
	// *******************************************************

	// Help->About
	else if (e.GetId() == MENU_HELP_ABOUT) {
		wxAboutDialogInfo info;
		info.SetName(_T("SLADE"));
		info.SetVersion(Global::version);
		info.SetWebSite(_T("http://slade.mancubus.net"));
		info.SetDescription(_T("It's a Doom Editor"));

		string year = wxNow().Right(4);
		info.SetCopyright(s_fmt(_T("(C) 2008-%s Simon Judd <sirjuddington@gmail.com>"), year.c_str()));

		wxAboutBox(info);
	}
}

/* MainWindow::onHTMLLinkClicked
 * Called when a link is clicked on the HTML Window, so that
 * external (http) links are opened in the default browser
 *******************************************************************/
void MainWindow::onHTMLLinkClicked(wxHtmlLinkEvent &e) {
	if (e.GetLinkInfo().GetHref().StartsWith(_T("http://")))
		wxLaunchDefaultBrowser(e.GetLinkInfo().GetHref());
	else
		html_startpage->OnLinkClicked(e.GetLinkInfo());
}

/* MainWindow::onClose
 * Called when the window is closed
 *******************************************************************/
void MainWindow::onClose(wxCloseEvent& e) {
	main_window_layout = m_mgr->SavePerspective();
	e.Skip();
	wxTheApp->Exit();
}
