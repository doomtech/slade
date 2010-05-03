
#include "Main.h"
#include "MainWindow.h"
#include "ArchiveManagerFrame.h"
#include "FileBrowserFrame.h"
#include <QtGui>
#include <QToolBar>

MainWindow::MainWindow() {
	// Set window defaults
	this->setWindowTitle("SLADE");
	this->resize(1024, 768);
	this->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

	// --- Add Archive Manager frame (docking) ---
	QDockWidget* dock = new QDockWidget("Archive Manager", this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
	w_archive_manager = new ArchiveManagerFrame(dock);
	dock->setWidget(w_archive_manager);
	addDockWidget(Qt::LeftDockWidgetArea, dock);

	// Set action to toggle the archive manager frame
	a_view_toggleArchiveManager = dock->toggleViewAction();

	// (Temporary) Add central area whatever
	QTextEdit* temp_middle = new QTextEdit(this);
	setCentralWidget(temp_middle);

	// Setup stuff
	setupActions();
	setupMenus();
	setupToolBars();
	setupStatusBar();
}

void MainWindow::setupActions() {
	// --- FILE MENU ---

	// New Wad
	a_file_newWad = new QAction("&Wad Archive", this);
	a_file_newWad->setStatusTip("Create a new Doom Wad archive");
	a_file_newWad->setToolTip("New Wad");
	a_file_newWad->setShortcut(QKeySequence("Ctrl+Shift+W"));
	a_file_newWad->setIcon(QIcon("icons/t_newarchive.png"));
	connect(a_file_newWad, SIGNAL(triggered()), this, SLOT(onFileNewWad()));

	// New Zip
	a_file_newZip = new QAction("&Zip Archive", this);
	a_file_newZip->setStatusTip("Create a new Zip archive (zip/pk3/jdf)");
	a_file_newZip->setToolTip("New Zip");
	a_file_newZip->setShortcut(QKeySequence("Ctrl+Shift+Z"));
	a_file_newZip->setIcon(QIcon("icons/t_newzip.png"));
	connect(a_file_newZip, SIGNAL(triggered()), this, SLOT(onFileNewZip()));

	// Open
	a_file_open = new QAction("&Open", this);
	a_file_open->setStatusTip("Open an existing archive");
	a_file_open->setToolTip("Open Archive");
	a_file_open->setShortcut(QKeySequence::Open);
	a_file_open->setIcon(QIcon("icons/t_open.png"));
	connect(a_file_open, SIGNAL(triggered()), this, SLOT(onFileOpen()));

	// Save All
	a_file_saveAll = new QAction("Save All", this);
	a_file_saveAll->setStatusTip("Save changes to all opened archives");
	a_file_saveAll->setToolTip("Save All");
	a_file_saveAll->setIcon(QIcon("icons/t_saveall.png"));

	// Close All
	a_file_closeAll = new QAction("Close All", this);
	a_file_closeAll->setStatusTip("Close all opened archives");
	a_file_closeAll->setToolTip("Close All");
	a_file_closeAll->setIcon(QIcon("icons/t_closeall.png"));

	// Exit
	a_file_exit = new QAction("E&xit", this);
	a_file_exit->setStatusTip("Exit SLADE");
	a_file_open->setToolTip("Exit");
	a_file_exit->setShortcut(QKeySequence::Quit);
	connect(a_file_exit, SIGNAL(triggered()), this, SLOT(close()));

	// --- EDITOR MENU ---

	// Set Base Resource Archive
	a_editor_setBaseResource = new QAction("Set &Base Resource Archive", this);
	a_editor_setBaseResource->setStatusTip("Set the base resource archive, which acts as the global program 'IWAD'");

	// Preferences
	a_editor_preferences = new QAction("Pr&eferences", this);
	a_editor_preferences->setStatusTip("Setup SLADE options and preferences");
	a_editor_preferences->setShortcut(QKeySequence::Preferences);

	// --- ARCHIVE MENU ---

	// New Entry
	a_archive_newEntry = new QAction("&Entry", this);
	a_archive_newEntry->setStatusTip("Create a new empty entry");
	a_archive_newEntry->setToolTip("New Entry");
	a_archive_newEntry->setIcon(QIcon("icons/t_newentry.png"));

	// New Directory
	a_archive_newDirectory = new QAction("&Directory", this);
	a_archive_newDirectory->setStatusTip("Create a new empty directory");
	a_archive_newDirectory->setToolTip("New Directory");
	a_archive_newDirectory->setIcon(QIcon("icons/t_newfolder.png"));

	// Import Files
	a_archive_importFiles = new QAction("&Import Files", this);
	a_archive_importFiles->setStatusTip("Import multiple files into the archive");
	a_archive_importFiles->setToolTip("Import Files");

	// Save
	a_archive_save = new QAction("&Save", this);
	a_archive_save->setStatusTip("Save changes to the current archive");
	a_archive_save->setToolTip("Save");
	a_archive_save->setShortcut(QKeySequence::Save);
	a_archive_save->setIcon(QIcon("icons/t_save.png"));

	// Save As
	a_archive_saveAs = new QAction("Save &As", this);
	a_archive_saveAs->setStatusTip("Save the current archive to a new file");
	a_archive_saveAs->setToolTip("Save As");
	a_archive_saveAs->setShortcut(QKeySequence::SaveAs);
	a_archive_saveAs->setIcon(QIcon("icons/t_saveas.png"));

	// Convert To
	a_archive_convertTo = new QAction("Convert &To", this);
	a_archive_convertTo->setStatusTip("Convert the current archive to a different format");
	a_archive_convertTo->setToolTip("Convert To");

	// Clean Up
	a_archive_cleanUp = new QAction("Clean &Up", this);
	a_archive_cleanUp->setStatusTip("Clean up the current archive by removing unused entries and data");
	a_archive_cleanUp->setToolTip("Clean Up");

	// Close
	a_archive_close = new QAction("&Close", this);
	a_archive_close->setStatusTip("Close the current archive");
	a_archive_close->setToolTip("Close");
	a_archive_close->setShortcut(QKeySequence::Close);
	a_archive_close->setIcon(QIcon("icons/t_close.png"));

	// --- ENTRY MENU ---

	// Rename
	a_entry_rename = new QAction("&Rename", this);
	a_entry_rename->setStatusTip("Rename selected entries");
	//a_entry_rename->setToolTop("Rename Entries");
	//a_entry_rename->setShortcut(QKeySequence::Close);
	a_entry_rename->setIcon(QIcon("icons/t_rename.png"));

	// Delete
	a_entry_delete = new QAction("&Delete", this);
	a_entry_delete->setStatusTip("Delete selected entries");
	//a_entry_delete->setToolTop("Delete Entries");
	//a_entry_delete->setShortcut(QKeySequence::Close);
	a_entry_delete->setIcon(QIcon("icons/t_delete.png"));

	// Move Up
	a_entry_moveUp = new QAction("Move Up", this);
	a_entry_moveUp->setStatusTip("Move selected entries up");
	//a_entry_moveUp->setToolTop("Move Up");
	//a_entry_moveUp->setShortcut(QKeySequence::Close);
	a_entry_moveUp->setIcon(QIcon("icons/t_up.png"));

	// Move Down
	a_entry_moveDown = new QAction("Move Down", this);
	a_entry_moveDown->setStatusTip("Move selected entries down");
	//a_entry_moveDown->setToolTop("Delete Entries");
	//a_entry_moveDown->setShortcut(QKeySequence::Close);
	a_entry_moveDown->setIcon(QIcon("icons/t_down.png"));

	// Convert To
	a_entry_convertTo = new QAction("Convert &To", this);
	a_entry_convertTo->setStatusTip("Convert selected entries to a different format/type");
	//a_entry_convertTo->setToolTop("Convert Entries");
	//a_entry_convertTo->setShortcut(QKeySequence::Close);
	//a_entry_convertTo->setIcon(QIcon("icons/t_delete.png"));

	// Import
	a_entry_import = new QAction("&Import", this);
	a_entry_import->setStatusTip("Import a file to the selected entries");
	//a_entry_import->setToolTop("Delete Entries");
	//a_entry_import->setShortcut(QKeySequence::Close);
	a_entry_import->setIcon(QIcon("icons/t_import.png"));

	// Export
	a_entry_export = new QAction("&Export", this);
	a_entry_export->setStatusTip("Export selected entries to files");
	//a_entry_export->setToolTop("Delete Entries");
	//a_entry_export->setShortcut(QKeySequence::Close);
	a_entry_export->setIcon(QIcon("icons/t_export.png"));

	// Export As
	a_entry_exportAs = new QAction("Export As", this);
	a_entry_exportAs->setStatusTip("Export selected entries to files as a different format/type");
	//a_entry_exportAs->setToolTop("Delete Entries");
	//a_entry_exportAs->setShortcut(QKeySequence::Close);
	//a_entry_exportAs->setIcon(QIcon("icons/t_export.png"));

	// --- VIEW MENU ---

	// Toggle Archive Manager
	a_view_toggleArchiveManager->setStatusTip("Toggle the Archive Manager window");
	a_view_toggleArchiveManager->setShortcut(QKeySequence("Ctrl+1"));

	// --- HELP MENU ---

	// Online Documentation
	a_help_onlineDocumentation = new QAction("Online &Documentation", this);
	a_help_onlineDocumentation->setStatusTip("View SLADE documentation online");

	// About
	a_help_about = new QAction("&About SLADE", this);
	a_help_about->setStatusTip("View information about SLADE");
}

void MainWindow::setupMenus() {
	// File menu
	QMenu* menu_file = new QMenu("&File");
	QMenu* menu_file_new = new QMenu("&New");
	menu_file_new->addAction(a_file_newWad);
	menu_file_new->addAction(a_file_newZip);
	menu_file->addMenu(menu_file_new);
	menu_file->addAction(a_file_open);
	menu_file->addSeparator();
	menu_file->addAction(a_archive_save);
	menu_file->addAction(a_archive_saveAs);
	menu_file->addAction(a_file_saveAll);
	menu_file->addSeparator();
	menu_file->addAction(a_archive_close);
	menu_file->addAction(a_file_closeAll);
	menu_file->addSeparator();
	menu_file->addAction(a_file_exit);
	menuBar()->addMenu(menu_file);

	// Editor menu
	QMenu* menu_editor = new QMenu("E&ditor");
	menu_editor->addAction(a_editor_setBaseResource);
	menu_editor->addAction(a_editor_preferences);
	menuBar()->addMenu(menu_editor);

	// Archive menu
	QMenu* menu_archive = new QMenu("&Archive");
	QMenu* menu_archive_new = new QMenu("&New");
	menu_archive_new->addAction(a_archive_newEntry);
	menu_archive_new->addAction(a_archive_newDirectory);
	menu_archive->addMenu(menu_archive_new);
	menu_archive->addAction(a_archive_importFiles);
	menu_archive->addSeparator();
	menu_archive->addAction(a_archive_convertTo);
	menu_archive->addAction(a_archive_cleanUp);
	menuBar()->addMenu(menu_archive);

	// Entry menu
	QMenu* menu_entry = new QMenu("&Entry");
	menu_entry->addAction(a_entry_rename);
	menu_entry->addAction(a_entry_delete);
	menu_entry->addAction(a_entry_moveUp);
	menu_entry->addAction(a_entry_moveDown);
	menu_entry->addAction(a_entry_convertTo);
	menu_entry->addAction(a_entry_import);
	menu_entry->addAction(a_entry_export);
	menu_entry->addAction(a_entry_exportAs);
	menuBar()->addMenu(menu_entry);

	// View menu
	QMenu* menu_view = new QMenu("&View");
	menu_view->addAction(a_view_toggleArchiveManager);
	menuBar()->addMenu(menu_view);

	// Help menu
	QMenu* menu_help = new QMenu("&Help");
	menu_help->addAction(a_help_onlineDocumentation);
	menu_help->addAction(a_help_about);
	menuBar()->addMenu(menu_help);
}

void MainWindow::setupToolBars() {
	// File toolbar
	QToolBar* tb_file = addToolBar("File Toolbar");
	tb_file->setIconSize(QSize(16, 16));
	tb_file->addAction(a_file_newWad);
	tb_file->addAction(a_file_newZip);
	tb_file->addAction(a_file_open);
	tb_file->addAction(a_archive_save);
	tb_file->addAction(a_archive_saveAs);
	tb_file->addAction(a_file_saveAll);
	tb_file->addAction(a_archive_close);
	tb_file->addAction(a_file_closeAll);

	// Archive toolbar
	QToolBar* tb_archive = addToolBar("Archive Toolbar");
	tb_archive->setIconSize(QSize(16, 16));
	tb_archive->addAction(a_archive_newEntry);
	tb_archive->addAction(a_archive_newDirectory);
	tb_archive->addAction(a_entry_rename);
	tb_archive->addAction(a_entry_delete);
	tb_archive->addAction(a_entry_moveUp);
	tb_archive->addAction(a_entry_moveDown);
	tb_archive->addAction(a_entry_import);
	tb_archive->addAction(a_entry_export);
}

void MainWindow::setupStatusBar() {
	statusBar()->showMessage("Ready");
}

MainWindow::~MainWindow() {
}

void MainWindow::onFileNewWad() {
	QMessageBox::critical(this, "SLADE", "Not Implemented");
}

void MainWindow::onFileNewZip() {
	QMessageBox::critical(this, "SLADE", "Not Implemented");
}

void MainWindow::onFileOpen() {
	QStringList files = QFileDialog::getOpenFileNames(this, "Open Archive(s)", "", "All Supported Archives (*.wad *.zip *.pk3 *.jdf);;Wad Archives (*.wad);;Zip Archives (*.zip *.pk3 *.jdf)");

	for (int a = 0; a < files.size(); a++)
		QMessageBox::information(this, "SLADE", s_fmt("Open file %s", chr(files[a])));
}
