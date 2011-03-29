
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
#include "BaseResourceChooser.h"
#include "PreferencesDialog.h"
#include "Tokenizer.h"
#include <wx/aboutdlg.h>
#include <wx/dnd.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
string main_window_layout = "";
MainWindow* MainWindow::instance = NULL;
CVAR(Bool, show_start_page, true, CVAR_SAVE);
CVAR(String, global_palette, "", CVAR_SAVE);


/*******************************************************************
 * MAINWINDOWDROPTARGET CLASS
 *******************************************************************
 Handles drag'n'drop of files on to the SLADE window
*/
class MainWindowDropTarget : public wxFileDropTarget {
public:
	MainWindowDropTarget(){}
	~MainWindowDropTarget(){}

	bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) {
		for (unsigned a = 0; a < filenames.size(); a++)
			theArchiveManager->openArchive(filenames[a]);

		return true;
	}
};


/*******************************************************************
 * MAINWINDOW CLASS FUNCTIONS
 *******************************************************************/

/* MainWindow::MainWindow
 * MainWindow class constructor
 *******************************************************************/
MainWindow::MainWindow()
: wxFrame((wxFrame *) NULL, -1, "SLADE", wxPoint(0, 0), wxSize(1024, 768)) {
	lasttipindex = 0;
	Maximize();
	setupLayout();
	SetDropTarget(new MainWindowDropTarget());
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


	// -- Editor Area --
	notebook_tabs = new wxAuiNotebook(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE|wxNO_BORDER|wxAUI_NB_WINDOWLIST_BUTTON|wxNB_FLAT);

	// Setup panel info & add panel
	p_inf.CenterPane();
	p_inf.Name("editor_area");
	p_inf.PaneBorder(false);
	m_mgr->AddPane(notebook_tabs, p_inf);

	// Create Start Page (temporary)
	html_startpage = new wxHtmlWindow(notebook_tabs, -1, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_NEVER, "startpage");
	html_startpage->SetName("startpage");
	if (show_start_page) {
		notebook_tabs->AddPage(html_startpage,"Start Page");
		notebook_tabs->SetPageBitmap(0, getIcon("i_logo"));
		createStartPage();
	}
	else
		html_startpage->Show(false);


	// -- Console Panel --
	ConsolePanel *panel_console = new ConsolePanel(this, -1);

	// Setup panel info & add panel
	p_inf.DefaultPane();
	p_inf.Float();
	p_inf.FloatingSize(600, 400);
	p_inf.FloatingPosition(100, 100);
	p_inf.MinSize(-1, 192);
	p_inf.Show(false);
	p_inf.Caption("Console");
	p_inf.Name("console");
	m_mgr->AddPane(panel_console, p_inf);


	// -- Archive Manager Panel --
	panel_archivemanager = new ArchiveManagerPanel(this, notebook_tabs);

	// Setup panel info & add panel
	p_inf.DefaultPane();
	p_inf.Left();
	p_inf.BestSize(192, 480);
	p_inf.Caption("Archive Manager");
	p_inf.Name("archive_manager");
	p_inf.Show(true);
	p_inf.Dock();
	m_mgr->AddPane(panel_archivemanager, p_inf);


	// -- Menu bar --
	wxMenuBar *menu = new wxMenuBar();

	// File menu
	wxMenu* fileNewMenu = new wxMenu("");
	fileNewMenu->Append(createMenuItem(fileNewMenu, MENU_FILE_NEWWAD,	"&Wad Archive\tCtrl+Shift+W",	"Create a new Doom Wad Archive",					"t_newarchive"));
	fileNewMenu->Append(createMenuItem(fileNewMenu, MENU_FILE_NEWZIP,	"&Zip Archive\tCtrl+Shift+Z",	"Create a new Zip Archive (zip/pk3/jdf)",			"t_newzip"));
	wxMenu* fileMenu = new wxMenu("");
	fileMenu->AppendSubMenu(fileNewMenu,	"&New",				"Create a new Archive");
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_OPEN,		"&Open\tCtrl+O",					"Open an existing Archive",							"t_open"));
	fileMenu->AppendSeparator();
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_SAVE,		"&Save\tCtrl+S",					"Save the currently open Archive",					"t_save"));
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_SAVEAS,		"Save &As...\tCtrl+Shift+S",		"Save the currently open Archive to a new file",	"t_saveas"));
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_SAVEALL,	"Save All",							"Save all open Archives",							"t_saveall"));
	fileMenu->AppendSubMenu(panel_archivemanager->recentFilesMenu(), "&Recent Files");
	fileMenu->AppendSeparator();
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_CLOSE,		"&Close\tCtrl+W",					"Close the currently open Archive",					"t_close"));
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_CLOSEALL,	"Close All",						"Close all open Archives",							"t_closeall"));
	fileMenu->AppendSeparator();
	fileMenu->Append(createMenuItem(fileMenu, MENU_FILE_EXIT,		"E&xit",							"Quit SLADE"));
	menu->Append(fileMenu, "&File");

	// Editor menu
	wxMenu* editorMenu = new wxMenu("");
	editorMenu->Append(createMenuItem(editorMenu, MENU_EDITOR_SETBASERESOURCE,	"Set &Base Resource Archive",	"Set the base resource archive, to act as the 'program IWAD'"));
	editorMenu->Append(createMenuItem(editorMenu, MENU_EDITOR_PREFERENCES,		"&Preferences...",				"Setup SLADE options and preferences"));
	menu->Append(editorMenu, "E&ditor");

	// Archive menu
	wxMenu* archiveNewMenu = new wxMenu("");
	archiveNewMenu->Append(createMenuItem(archiveNewMenu,	MENU_ARCHIVE_NEWENTRY,			"&Entry",					"Create a new empty entry",		"t_newentry"));
	archiveNewMenu->Append(createMenuItem(archiveNewMenu,	MENU_ARCHIVE_NEWDIRECTORY,		"&Directory",				"Create a new empty directory",	"t_newfolder"));
	wxMenu* archiveMenu = new wxMenu("");
	archiveMenu->AppendSubMenu(archiveNewMenu, "&New");
	archiveMenu->Append(createMenuItem(archiveMenu,			MENU_ARCHIVE_IMPORTFILES,		"&Import Files",			"Import multiple files into the archive",	"t_importfiles"));
	archiveMenu->AppendSeparator();
	archiveMenu->Append(createMenuItem(archiveMenu,			MENU_ARCHIVE_TEXEDITOR,			"&Texture Editor",			"Open the texture editor for the current archive",	"t_texeditor"));
	//archiveMenu->Append(createMenuItem(archiveMenu,	MENU_ARCHIVE_CONVERTTO,		"&Convert To...",	"Convert the current archive to a different format"));
	//archiveMenu->Append(createMenuItem(archiveMenu,	MENU_ARCHIVE_CLEANUP,		"Clean &Up",		"Clean up the current archive by removing unused entries and data"));
	wxMenu* archiveCleanMenu = new wxMenu("");
	archiveCleanMenu->Append(createMenuItem(archiveCleanMenu, MENU_ARCHIVE_CLEAN_PATCHES, 	"Remove Unused &Patches",	"Removes any unused patches, and their associated entries"));
	archiveMenu->AppendSubMenu(archiveCleanMenu, "Clean &Up");
	menu->Append(archiveMenu, "&Archive");

	// Entry menu
	wxMenu* entryMenu = new wxMenu("");
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_RENAME,		"Rename",			"Rename the selected entries",													"t_rename"));
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_DELETE,		"Delete",			"Delete the selected entries",													"t_delete"));
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_REVERT,		"Revert",			"Reverts any modifications made to the selected entries since the last save"));
	entryMenu->AppendSeparator();
	entryMenu->Append(createMenuItem(entryMenu,	MENU_ENTRY_CUT,			"Cut",				"Cut the selected entries"));
	entryMenu->Append(createMenuItem(entryMenu,	MENU_ENTRY_COPY,		"Copy",				"Copy the selected entries"));
	entryMenu->Append(createMenuItem(entryMenu,	MENU_ENTRY_PASTE,		"Paste",			"Paste the previously cut/copied entries"));
	entryMenu->AppendSeparator();
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_MOVEUP,		"Move Up",			"Move the selected entries up",													"t_up"));
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_MOVEDOWN,	"Move Down",		"Move the selected entries down",												"t_down"));
	//entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_CONVERTTO,	"Convert To...",	"Convert selected entries to a different format/type"));
	entryMenu->AppendSeparator();
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_IMPORT,		"Import",			"Import a file to the selected entry",											"t_import"));
	entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_EXPORT,		"Export",			"Export the selected entries to files",											"t_export"));
	//entryMenu->Append(createMenuItem(entryMenu, MENU_ENTRY_EXPORTAS,	"Export As...",		"Export the selected entries to files as a different format/type"));
	entryMenu->AppendSeparator();
	entryMenu->Append(createMenuItem(entryMenu,	MENU_ENTRY_BOOKMARK,	"Bookmark",			"Bookmark the current entry"));
	menu->Append(entryMenu, "&Entry");

	// View menu
	wxMenu* viewMenu = new wxMenu("");
	viewMenu->Append(MENU_VIEW_ARCHIVEMANAGER,	"&Archive Manager\tCtrl+1",	"Toggle the archive manager");
	viewMenu->Append(MENU_VIEW_CONSOLE,			"&Console\tCtrl+2",			"Toggle the console");
	menu->Append(viewMenu, "&View");

	// Help menu
	wxMenu* helpMenu = new wxMenu("");
	helpMenu->Append(MENU_HELP_ONLINEDOCUMENTATION,	"Online &Documentation",	"View SLADE documentation online");
	helpMenu->Append(MENU_HELP_ABOUT,				"&About",					"Information about SLADE");
	menu->Append(helpMenu, "&Help");

	// Set the menu
	SetMenuBar(menu);



	// -- Toolbars --

	// Create File toolbar
	wxAuiToolBar* tb_file = new wxAuiToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	tb_file->AddTool(MENU_FILE_NEWWAD, "New Wad", getIcon("t_newarchive"), "New Wad Archive");
	tb_file->AddTool(MENU_FILE_NEWZIP, "New Zip", getIcon("t_newzip"), "New Zip Archive");
	tb_file->AddTool(MENU_FILE_OPEN, "Open", getIcon("t_open"), "Open");
	tb_file->AddTool(MENU_FILE_SAVE, "Save", getIcon("t_save"), "Save");
	tb_file->AddTool(MENU_FILE_SAVEAS, "Save As", getIcon("t_saveas"), "Save As");
	tb_file->AddTool(MENU_FILE_SAVEALL, "Save All", getIcon("t_saveall"), "Save All");
	tb_file->AddTool(MENU_FILE_CLOSE, "Close", getIcon("t_close"), "Close");
	tb_file->AddTool(MENU_FILE_CLOSEALL, "Close All", getIcon("t_closeall"), "Close All");
	tb_file->Realize();

	// Create Archive toolbar
	wxAuiToolBar* tb_archive = new wxAuiToolBar(this, -1);
	tb_archive->AddTool(MENU_ARCHIVE_NEWENTRY, "New", getIcon("t_newentry"), "New Entry");
	tb_archive->AddTool(MENU_ARCHIVE_NEWDIRECTORY, "Directory", getIcon("t_newfolder"), "New Directory");
	tb_archive->AddTool(MENU_ARCHIVE_IMPORTFILES, "Import Files", getIcon("t_importfiles"), "Import Files to Archive");
	tb_archive->AddTool(MENU_ARCHIVE_TEXEDITOR, "Texture Editor", getIcon("t_texeditor"), "Open TEXTUREx Editor");
	tb_archive->Realize();

	// Create Entry toolbar
	wxAuiToolBar* tb_entry = new wxAuiToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	tb_entry->AddTool(MENU_ENTRY_RENAME,		"Rename",		getIcon("t_rename"),	"Rename");
	tb_entry->AddTool(MENU_ENTRY_DELETE,		"Delete",		getIcon("t_delete"),	"Delete");
	tb_entry->AddTool(MENU_ENTRY_IMPORT,		"Import",		getIcon("t_import"),	"Import");
	tb_entry->AddTool(MENU_ENTRY_EXPORT,		"Export",		getIcon("t_export"),	"Export");
	tb_entry->AddTool(MENU_ENTRY_MOVEUP,		"Move Up",		getIcon("t_up"),		"Move Up");
	tb_entry->AddTool(MENU_ENTRY_MOVEDOWN,		"Move Down",	getIcon("t_down"),		"Move Down");
	tb_entry->Realize();

	// Create Base Resource Archive toolbar
	wxAuiToolBar* tb_bra = new wxAuiToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	BaseResourceChooser* brc = new BaseResourceChooser(tb_bra);
	tb_bra->AddLabel(-1, "Base Resource:");
	tb_bra->AddControl(brc);
	tb_bra->AddTool(MENU_EDITOR_SETBASERESOURCE, "...", getIcon("t_settings"), "Setup Base Resource Archive paths");
	tb_bra->Realize();

	// Create Palette Chooser toolbar
	wxAuiToolBar* tb_pal = new wxAuiToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	palette_chooser = new PaletteChooser(tb_pal, -1);
	palette_chooser->selectPalette(global_palette);
	tb_pal->AddLabel(-1, "Palette:");
	tb_pal->AddControl(palette_chooser);
	tb_pal->Realize();

	// Setup panel info & add toolbar panels
	m_mgr->AddPane(tb_file, wxAuiPaneInfo().ToolbarPane().Top().Name("tb_file").CloseButton(false));					// File toolbar
	m_mgr->AddPane(tb_archive, wxAuiPaneInfo().ToolbarPane().Top().Name("tb_archive").Position(1).CloseButton(false));	// Archive toolbar
	m_mgr->AddPane(tb_entry, wxAuiPaneInfo().ToolbarPane().Top().Name("tb_entry").Position(2).CloseButton(false));		// Entry toolbar
	m_mgr->AddPane(tb_bra, wxAuiPaneInfo().ToolbarPane().Top().Name("tb_bra").Position(3).CloseButton(false));			// Base Resource Archive toolbar
	m_mgr->AddPane(tb_pal, wxAuiPaneInfo().ToolbarPane().Top().Name("tb_pal").Position(4).CloseButton(false));			// Palette toolbar


	// -- Status Bar --
	CreateStatusBar(3);


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
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::onMenuItemClicked, this);
	Bind(wxEVT_CLOSE_WINDOW, &MainWindow::onClose, this);
	Bind(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, &MainWindow::onTabChanged, this);
}

/* MainWindow::createStartPage
 * Builds the HTML start page and loads it into the html viewer
 * (start page tab)
 *******************************************************************/
void MainWindow::createStartPage() {
	// Get relevant resource entries
	Archive* res_archive = theArchiveManager->programResourceArchive();
	if (!res_archive)
		return;
	ArchiveEntry* entry_html = res_archive->entryAtPath("html/startpage.htm");
	ArchiveEntry* entry_logo = res_archive->entryAtPath("logo.png");
	ArchiveEntry* entry_tips = res_archive->entryAtPath("tips.txt");

	// Can't do anything without html entry
	if (!entry_html) {
		html_startpage->SetPage("<html><head><title>SLADE</title></head><body><center><h1>Something is wrong with slade.pk3 :(</h1><center></body></html>");
		return;
	}

	// Get html as string
	string html = wxString::FromAscii((const char*)(entry_html->getData()), entry_html->getSize());

	// Generate tip of the day string
	string tip = "Tip";
	if (entry_tips) {
		Tokenizer tz;
		tz.openMem((const char*)entry_tips->getData(), entry_tips->getSize(), entry_tips->getName());
		srand(wxGetLocalTime());
		int numtips = tz.getInteger();
		if (numtips < 2) // Needs at least two choices or it's kinda pointless.
			tip = "Did you know? Something is wrong with the tips.txt file in your slade.pk3.";
		else {
			int tipindex = 0;
			// Don't show same tip twice in a row
			do { tipindex = 1 + (rand() % numtips); } while (tipindex == lasttipindex);
			lasttipindex = tipindex;
			for (int a = 0; a < tipindex; a++)
				tip = tz.getToken();
		}
	}

	// Generate recent files string
	string recent;
	for (unsigned a = 0; a < 4; a++) {
		if (a >= theArchiveManager->numRecentFiles())
			break;	// No more recent files

		// Add line break if needed
		if (a > 0) recent += "<br/>\n";

		// Add recent file link
		recent += S_FMT("<a href=\"recent://%d\">%s</a>", a, theArchiveManager->recentFile(a));
	}

	// Insert tip and recent files into html
	html.Replace("#recent#", recent);
	html.Replace("#totd#", tip);

	// Write html and images to temp folder
	if (entry_logo) entry_logo->exportFile(appPath("logo.png", DIR_TEMP));
	string html_file = appPath("startpage.htm", DIR_TEMP);
	wxFile outfile(html_file, wxFile::write);
	outfile.Write(html);
	outfile.Close();

	// Load page
	html_startpage->LoadPage(html_file);

	// Clean up
	wxRemoveFile(html_file);
	wxRemoveFile(appPath("logo.png", DIR_TEMP));
}

/* MainWindow::exitProgram
 * Attempts to exit the program. Only fails if an unsaved archive is
 * found and the user cancels the exit
 *******************************************************************/
bool MainWindow::exitProgram() {
	// Close all archives
	if (!panel_archivemanager->closeAll())
		return false;

	// Save current layout
	main_window_layout = m_mgr->SavePerspective();

	// Save selected palette
	global_palette = palette_chooser->GetStringSelection();

	// Exit application
	wxTheApp->Exit();

	return true;
}

/* MainWindow::getCurrentArchive
 * Returns the currently open archive (ie the current tab's archive,
 * if any)
 *******************************************************************/
Archive* MainWindow::getCurrentArchive() {
	return panel_archivemanager->currentArchive();
}

/* MainWindow::getCurrentEntry
 * Returns the currently open entry (current tab -> current entry
 * panel)
 *******************************************************************/
ArchiveEntry* MainWindow::getCurrentEntry() {
	return panel_archivemanager->currentEntry();
}

/* MainWindow::getCurrentEntrySelection
 * Returns a list of all currently selected entries, in the current
 * archive panel
 *******************************************************************/
vector<ArchiveEntry*> MainWindow::getCurrentEntrySelection() {
	return panel_archivemanager->currentEntrySelection();
}

/* MainWindow::openTextureEditor
 * Opens the texture editor for the current archive tab
 *******************************************************************/
void MainWindow::openTextureEditor(Archive* archive) {
	panel_archivemanager->openTextureTab(theArchiveManager->archiveIndex(archive));
}

/* MainWindow::openEntry
 * (Unimplemented) open [entry] in the editor, opening the correct
 * archive tab and entry panel
 *******************************************************************/
void MainWindow::openEntry(ArchiveEntry* entry) {
}

void MainWindow::addCustomMenu(wxMenu* menu, string title) {
	// Insert custom menus between 'Entry' and 'View' menus
	if (GetMenuBar()->FindMenu(title) == wxNOT_FOUND) {
		GetMenuBar()->Insert(4, menu, title);
		GetMenuBar()->Refresh();
	}
}

void MainWindow::removeCustomMenu(string title) {
	// Get custom menu index
	int index = GetMenuBar()->FindMenu(title);

	// Remove it
	if (index != wxNOT_FOUND) {
		GetMenuBar()->Remove(index);
		GetMenuBar()->Refresh();
	}
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

	// File->Exit
	if (e.GetId() == MENU_FILE_EXIT)
		Destroy();


	// *******************************************************
	// EDITOR MENU
	// *******************************************************

	// Editor->Set Base Resource Archive
	else if (e.GetId() == MENU_EDITOR_SETBASERESOURCE) {
		wxDialog dialog_ebr(this, -1, "Edit Base Resource Archives", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);
		BaseResourceArchivesPanel brap(&dialog_ebr);

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(&brap, 1, wxEXPAND|wxALL, 4);

		sizer->Add(dialog_ebr.CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxDOWN, 4);

		dialog_ebr.SetSizer(sizer);
		dialog_ebr.Layout();
		dialog_ebr.SetInitialSize(wxSize(-1, 300));
		if (dialog_ebr.ShowModal() == wxID_OK)
			theArchiveManager->openBaseResource(brap.getSelectedPath());
	}

	// Editor->Preferences
	if (e.GetId() == MENU_EDITOR_PREFERENCES) {
		PreferencesDialog pd(this);
		if (pd.ShowModal() == wxID_OK)
			pd.applyPreferences();
		panel_archivemanager->refreshAllTabs();
	}


	// *******************************************************
	// VIEW MENU
	// *******************************************************

	// View->Archive Manager
	else if (e.GetId() == MENU_VIEW_ARCHIVEMANAGER) {
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
		p_inf.MinSize(200, 128);
		m_mgr->Update();
	}


	// *******************************************************
	// HELP MENU
	// *******************************************************

	// Help->About
	else if (e.GetId() == MENU_HELP_ABOUT) {
		wxAboutDialogInfo info;
		info.SetName("SLADE");
		info.SetVersion("v" + Global::version);
		info.SetWebSite("http://slade.mancubus.net");
		info.SetDescription("It's a Doom Editor");

		// Set icon
		string icon_filename = appPath("slade.ico", DIR_TEMP);
		theArchiveManager->programResourceArchive()->getEntry("slade.ico")->exportFile(icon_filename);
		info.SetIcon(wxIcon(icon_filename, wxBITMAP_TYPE_ICO));
		wxRemoveFile(icon_filename);

		string year = wxNow().Right(4);
		info.SetCopyright(S_FMT("(C) 2008-%s Simon Judd <sirjuddington@gmail.com>", year.c_str()));

		wxAboutBox(info);
	}

	// Help->Online Documentation
	else if (e.GetId() == MENU_HELP_ONLINEDOCUMENTATION)
		wxLaunchDefaultBrowser("http://slade-editor.wikia.com");

	// Not handled here, sent to ArchiveManagerPanel
	else
		panel_archivemanager->handleAction(e.GetId());
}

/* MainWindow::onHTMLLinkClicked
 * Called when a link is clicked on the HTML Window, so that
 * external (http) links are opened in the default browser
 *******************************************************************/
void MainWindow::onHTMLLinkClicked(wxHtmlLinkEvent &e) {
	string href = e.GetLinkInfo().GetHref();

	if (href.StartsWith("http://"))
		wxLaunchDefaultBrowser(e.GetLinkInfo().GetHref());
	else if (href.StartsWith("recent://")) {
		// Recent file
		string rs = href.Right(1);
		unsigned long index = 0;
		rs.ToULong(&index);

		panel_archivemanager->handleAction(MENU_RECENT_1 + index);
	}
	else if (href.StartsWith("action://")) {
		// Action
		if (href.EndsWith("open"))
			panel_archivemanager->handleAction(MENU_FILE_OPEN);
		else if (href.EndsWith("newwad"))
			panel_archivemanager->handleAction(MENU_FILE_NEWWAD);
		else if (href.EndsWith("newzip"))
			panel_archivemanager->handleAction(MENU_FILE_NEWZIP);
	}
	else
		html_startpage->OnLinkClicked(e.GetLinkInfo());
}

/* MainWindow::onClose
 * Called when the window is closed
 *******************************************************************/
void MainWindow::onClose(wxCloseEvent& e) {
	if (!exitProgram())
		e.Veto();
}

/* MainWindow::onTabChanged
 * Called when the current tab is changed
 *******************************************************************/
void MainWindow::onTabChanged(wxAuiNotebookEvent& e) {
	// Get current page
	wxWindow* page = notebook_tabs->GetPage(notebook_tabs->GetSelection());

	// If start page is selected, refresh it
	if (page->GetName() == "startpage") {
		createStartPage();
		SetStatusText("", 1);
		SetStatusText("", 2);
	}

	// Continue
	e.Skip();
}
