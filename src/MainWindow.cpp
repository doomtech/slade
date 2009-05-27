
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
#include <wx/aui/aui.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
string aui_perspective = _T("");


/* MainWindow::MainWindow
 * MainWindow class constructor
 *******************************************************************/
MainWindow::MainWindow()
: wxFrame((wxFrame *) NULL, -1, _T("SLADE"), wxPoint(0, 0), wxSize(800, 600)) {
	setupLayout();
	Maximize();
}

/* MainWindow::~MainWindow
 * MainWindow class destructor
 *******************************************************************/
MainWindow::~MainWindow() {
}

wxImage get_toolbar_icon(string name, int type) {
	// Init
	wxImage image;

	// Get the needed entry from the SLADE resource pk3
	Archive* resource_pk3 = ArchiveManager::getInstance().resourceArchive();
	string entry_path = _T("toolbar_icons/") + name;
	ArchiveEntry* entry = resource_pk3->getEntry(entry_path);

	if (entry) {
		// Export entry data to a temporary file
		entry->exportFile(_T("sladetemp"));

		// Load the data to a wxImage
		image.LoadFile(_T("sladetemp"), type);

		// Delete the temporary file
		wxRemoveFile(_T("sladetemp"));
	}

	// Return the image, loaded or not
	return image;
}

/* MainWindow::setupLayout
 * Sets up the wxWidgets window layout
 *******************************************************************/
void MainWindow::setupLayout() {
	// Create the wxAUI manager & related things
	wxAuiManager *m_mgr = new wxAuiManager(this);
	wxAuiPaneInfo p_inf;


	// -- Menu bar --
	wxMenuBar *menu = new wxMenuBar();

	// File menu
	wxMenu* fileMenu = new wxMenu(_(""));
	fileMenu->Append(MENU_FILE_NEW,			_("&New\tCtrl+N"),		_("Create a new Archive"));
	fileMenu->AppendSeparator();
    fileMenu->Append(MENU_FILE_OPEN,		_("&Open\tCtrl+O"),		_("Open an existing Archive"));
	fileMenu->AppendSeparator();
	fileMenu->Append(MENU_FILE_SAVE,		_("&Save\tCtrl+S"),		_("Save the currently open Archive"));
	fileMenu->Append(MENU_FILE_SAVEAS,		_("Save &As..."),		_("Save the currently open Archive to a new file"));
	fileMenu->Append(MENU_FILE_SAVEALL,		_("Save All"),			_("Save all open Archives"));
	fileMenu->AppendSeparator();
	fileMenu->Append(MENU_FILE_CLOSE,		_("&Close"),			_("Close the currently open Archive"));
	fileMenu->Append(MENU_FILE_CLOSEALL,	_("Close All"),			_("Close all open Archives"));
	fileMenu->AppendSeparator();
    fileMenu->Append(MENU_FILE_QUIT,		_("&Quit\tCtrl+X"),		_("Quit SLADE"));
	menu->Append(fileMenu, _("&File"));

	// Entry menu
	wxMenu* entryMenu = new wxMenu(_T(""));
	entryMenu->Append(MENU_ENTRY_NEW,				_("New"),					_("Create a new empty entry"));
	entryMenu->Append(MENU_ENTRY_NEWFROMFILE,		_("New from File"),			_("Create a new entry with data from a file"));
	entryMenu->AppendSeparator();
	entryMenu->Append(MENU_ENTRY_RENAME,			_("Rename\tCtrl+R"),		_("Rename the selected entries"));
	entryMenu->Append(MENU_ENTRY_DELETE,			_("Delete\tDelete"),		_("Delete the selected entries"));
	entryMenu->AppendSeparator();
	entryMenu->Append(MENU_ENTRY_IMPORT,			_("Import\tCtrl+I"),		_("Import a file to the selected entry"));
	entryMenu->Append(MENU_ENTRY_EXPORT,			_("Export\tCtrl+E"),		_("Export the selected entries to files"));
	entryMenu->Append(MENU_ENTRY_EXPORTWAD,			_("Export as Wad\tCtrl+W"),	_("Export the selected entries to a new Wad Archive"));
	entryMenu->AppendSeparator();
	entryMenu->Append(MENU_ENTRY_MOVEUP,			_("Move Up\tCtrl+U"),		_("Move the selected entries up"));
	entryMenu->Append(MENU_ENTRY_MOVEDOWN,			_("Move Down\tCtrl+D"),		_("Move the selected entries down"));
	menu->Append(entryMenu, _("&Entry"));

	// View menu
	wxMenu* viewMenu = new wxMenu(_T(""));
	viewMenu->Append(MENU_VIEW_MANAGER,		_("&Archive Manager\tCtrl+1"),	_T("Toggle the archive manager"));
	viewMenu->Append(MENU_VIEW_CONSOLE,		_("&Console\tCtrl+2"),			_T("Toggle the console"));
	menu->Append(viewMenu, _("&View"));

	// Set the menu
	SetMenuBar(menu);



	// -- Toolbars --

	// Create File toolbar
	wxAuiToolBar* tb_file = new wxAuiToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	tb_file->AddTool(MENU_FILE_NEW, _T("New"), wxBitmap(get_toolbar_icon(_T("newarchive.png"), wxBITMAP_TYPE_PNG)), _T("New Archive"));
	tb_file->AddTool(MENU_FILE_OPEN, _T("Open"), wxBitmap(get_toolbar_icon(_T("open.png"), wxBITMAP_TYPE_PNG)), _T("Open"));
	tb_file->AddTool(MENU_FILE_SAVE, _T("Save"), wxBitmap(get_toolbar_icon(_T("save.png"), wxBITMAP_TYPE_PNG)), _T("Save"));
	tb_file->AddTool(MENU_FILE_SAVEAS, _T("Save As"), wxBitmap(get_toolbar_icon(_T("saveas.png"), wxBITMAP_TYPE_PNG)), _T("Save As"));
	tb_file->AddTool(MENU_FILE_SAVEALL, _T("Save All"), wxBitmap(get_toolbar_icon(_T("saveall.png"), wxBITMAP_TYPE_PNG)), _T("Save All"));
	tb_file->AddTool(MENU_FILE_CLOSE, _T("Close"), wxBitmap(get_toolbar_icon(_T("close.png"), wxBITMAP_TYPE_PNG)), _T("Close"));
	tb_file->AddTool(MENU_FILE_CLOSEALL, _T("Close All"), wxBitmap(get_toolbar_icon(_T("closeall.png"), wxBITMAP_TYPE_PNG)), _T("Close All"));
	tb_file->Realize();

	// Create Entry toolbar
	wxAuiToolBar* tb_entry = new wxAuiToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	tb_entry->AddTool(MENU_ENTRY_NEW, _T("New"), wxBitmap(get_toolbar_icon(_T("newentry.png"), wxBITMAP_TYPE_PNG)), _T("New Entry"));
	tb_entry->AddTool(MENU_ENTRY_RENAME, _T("Rename"), wxBitmap(get_toolbar_icon(_T("temp.png"), wxBITMAP_TYPE_PNG)), _T("Rename"));
	tb_entry->AddTool(MENU_ENTRY_DELETE, _T("Delete"), wxBitmap(get_toolbar_icon(_T("temp.png"), wxBITMAP_TYPE_PNG)), _T("Delete"));
	tb_entry->AddTool(MENU_ENTRY_IMPORT, _T("Import"), wxBitmap(get_toolbar_icon(_T("temp.png"), wxBITMAP_TYPE_PNG)), _T("Import"));
	tb_entry->AddTool(MENU_ENTRY_EXPORT, _T("Export"), wxBitmap(get_toolbar_icon(_T("temp.png"), wxBITMAP_TYPE_PNG)), _T("Export"));
	tb_entry->AddTool(MENU_ENTRY_MOVEUP, _T("Move Up"), wxBitmap(get_toolbar_icon(_T("up.png"), wxBITMAP_TYPE_PNG)), _T("Move Up"));
	tb_entry->AddTool(MENU_ENTRY_MOVEDOWN, _T("Move Down"), wxBitmap(get_toolbar_icon(_T("down.png"), wxBITMAP_TYPE_PNG)), _T("Move Down"));
	tb_entry->Realize();

	// Setup panel info & add toolbar panels
	// File toolbar
	p_inf.ToolbarPane();
	p_inf.Top();
	m_mgr->AddPane(tb_file, p_inf);

	// Entry toolbar
	p_inf.ToolbarPane();
	p_inf.Top();
	p_inf.Position(1);
	m_mgr->AddPane(tb_entry, p_inf);


	// -- Editor Area --
	notebook_tabs = new wxAuiNotebook(this, -1, wxDefaultPosition, wxDefaultSize);

	// Setup panel info & add panel
	p_inf.CenterPane();
	m_mgr->AddPane(notebook_tabs, p_inf);

	// Create Start Page (temporary)
	html_startpage = new wxHtmlWindow(notebook_tabs, HTML_WINDOW);
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


	// Finalize
	m_mgr->Update();
	Layout();
}

/* MainWindow::setMockLayout
 * wxAUI testing stuff
 *******************************************************************/
void MainWindow::setMockLayout() {
	/*
	wxAuiManager *m_mgr = new wxAuiManager(this);


	// Menu bar
	wxMenuBar *menu = new wxMenuBar();

	// File menuGets the player's strafe vector
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
	EVT_HTML_LINK_CLICKED(HTML_WINDOW, MainWindow::onHTMLLinkClicked)

	// MENU

	// File Menu
	EVT_MENU(MENU_FILE_OPEN, MainWindow::onFileOpen)
	EVT_MENU(MENU_FILE_QUIT, MainWindow::onFileQuit)
	EVT_MENU(MENU_FILE_SAVE, MainWindow::onFileSave)
	EVT_MENU(MENU_FILE_SAVEAS, MainWindow::onFileSaveAs)
	EVT_MENU(MENU_FILE_CLOSE, MainWindow::onFileClose)

	// Entry menu
	EVT_MENU(MENU_ENTRY_NEW, MainWindow::onEntryNew)
	EVT_MENU(MENU_ENTRY_NEWFROMFILE, MainWindow::onEntryNewFromFile)
	EVT_MENU(MENU_ENTRY_RENAME, MainWindow::onEntryRename)
	EVT_MENU(MENU_ENTRY_DELETE, MainWindow::onEntryDelete)
	EVT_MENU(MENU_ENTRY_IMPORT, MainWindow::onEntryImport)
	EVT_MENU(MENU_ENTRY_EXPORT, MainWindow::onEntryExport)
	EVT_MENU(MENU_ENTRY_EXPORTWAD, MainWindow::onEntryExportWad)
	EVT_MENU(MENU_ENTRY_MOVEUP, MainWindow::onEntryMoveUp)
	EVT_MENU(MENU_ENTRY_MOVEDOWN, MainWindow::onEntryMoveDown)

	// View Menu
	EVT_MENU(MENU_VIEW_MANAGER, MainWindow::onViewArchiveManager)
	EVT_MENU(MENU_VIEW_CONSOLE, MainWindow::onViewConsole)
END_EVENT_TABLE()

/* MainWindow::onFileOpen
 * File->Open menu item event handler.
 *******************************************************************/
void MainWindow::onFileOpen(wxCommandEvent &e) {
	// Open a file browser dialog that allows multiple selection
	// and filters by wad, zip and pk3 file extensions
	wxFileDialog *dialog_open = new wxFileDialog(this, _T("Choose file(s) to open"), wxEmptyString, wxEmptyString,
			_T("Any Supported File (*.wad; *.zip; *.pk3; *.jdf)|*.wad;*.zip;*.pk3;*.jdf|Doom Wad files (*.wad)|*.wad|Zip files (*.zip)|*.zip|Pk3 (zip) files (*.pk3)|*.pk3|JDF (zip) files (*.jdf)|*.jdf"),
			wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_open->ShowModal() == wxID_OK) {
		// Get an array of selected filenames
		wxArrayString files;
		dialog_open->GetPaths(files);

		// Send it to the Archive Manager Panel
		panel_archivemanager->openFiles(files);
	}
}

/* MainWindow::onFileQuit
 * File->Quit menu item event handler.
 *******************************************************************/
void MainWindow::onFileQuit(wxCommandEvent &e) {
	wxTheApp->ExitMainLoop();
}

/* MainWindow::onFileSave
 * File->Save menu item event handler.
 *******************************************************************/
void MainWindow::onFileSave(wxCommandEvent& e) {
	panel_archivemanager->saveCurrent();
}

/* MainWindow::onFileSaveAs
 * File->Save As menu item event handler.
 *******************************************************************/
void MainWindow::onFileSaveAs(wxCommandEvent& e) {
	panel_archivemanager->saveCurrentAs();
}

/* MainWindow::onFileClose
 * File->Close menu item event handler.
 *******************************************************************/
void MainWindow::onFileClose(wxCommandEvent& e) {
	panel_archivemanager->closeCurrent();
}

/* MainWindow::onEntryNew
 * Entry->New menu item event handler.
 *******************************************************************/
void MainWindow::onEntryNew(wxCommandEvent& e) {
	panel_archivemanager->newEntry();
}

/* MainWindow::onEntryNewFromFile
 * Entry->New from File menu item event handler.
 *******************************************************************/
void MainWindow::onEntryNewFromFile(wxCommandEvent& e) {
	panel_archivemanager->newEntryFromFile();
}

/* MainWindow::onEntryRename
 * Entry->Rename menu item event handler.
 *******************************************************************/
void MainWindow::onEntryRename(wxCommandEvent& e) {
	panel_archivemanager->renameEntry();
}

/* MainWindow::onEntryDelete
 * Entry->Delete menu item event handler.
 *******************************************************************/
void MainWindow::onEntryDelete(wxCommandEvent& e) {
	panel_archivemanager->deleteEntry();
}

/* MainWindow::onEntryImport
 * Entry->Import menu item event handler.
 *******************************************************************/
void MainWindow::onEntryImport(wxCommandEvent& e) {
	panel_archivemanager->importEntry();
}

/* MainWindow::onEntryExport
 * Entry->Export menu item event handler.
 *******************************************************************/
void MainWindow::onEntryExport(wxCommandEvent& e) {
	panel_archivemanager->exportEntry();
}

/* MainWindow::onEntryExportWad
 * Entry->Export As Wad menu item event handler.
 *******************************************************************/
void MainWindow::onEntryExportWad(wxCommandEvent& e) {
	panel_archivemanager->exportEntryWad();
}

/* MainWindow::onEntryMoveUp
 * Entry->Move Up menu item event handler.
 *******************************************************************/
void MainWindow::onEntryMoveUp(wxCommandEvent& e) {
	panel_archivemanager->moveUp();
}

/* MainWindow::onEntryMoveDown
 * Entry->Move Down menu item event handler.
 *******************************************************************/
void MainWindow::onEntryMoveDown(wxCommandEvent& e) {
	panel_archivemanager->moveDown();
}

/* MainWindow::onViewArchiveManager
 * View->Archive Manager menu item event handler.
 *******************************************************************/
void MainWindow::onViewArchiveManager(wxCommandEvent &e) {
	wxAuiManager *m_mgr = wxAuiManager::GetManager(panel_archivemanager);
	wxAuiPaneInfo& p_inf = m_mgr->GetPane(_T("archive_manager"));
	p_inf.Show(!p_inf.IsShown());
	m_mgr->Update();
}

/* MainWindow::onViewConsole
 * View->Console menu item event handler.
 *******************************************************************/
void MainWindow::onViewConsole(wxCommandEvent &e) {
	wxAuiManager *m_mgr = wxAuiManager::GetManager(panel_archivemanager);
	wxAuiPaneInfo& p_inf = m_mgr->GetPane(_T("console"));
	p_inf.Show(!p_inf.IsShown());
	m_mgr->Update();
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
