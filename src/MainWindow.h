
#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include <QMainWindow>
#include "ArchiveManagerFrame.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

private:
	// Setup functions
	void	setupActions();
	void	setupMenus();
	void	setupToolBars();
	void	setupStatusBar();

	// Child widgets
	ArchiveManagerFrame*	w_archive_manager;

	// Actions
	QAction*	a_file_newWad;
	QAction*	a_file_newZip;
	QAction*	a_file_open;
	QAction*	a_file_saveAll;
	QAction*	a_file_closeAll;
	QAction*	a_file_exit;
	QAction*	a_editor_setBaseResource;
	QAction*	a_editor_preferences;
	QAction*	a_archive_newEntry;
	QAction*	a_archive_newDirectory;
	QAction*	a_archive_importFiles;
	QAction*	a_archive_save;
	QAction*	a_archive_saveAs;
	QAction*	a_archive_convertTo;
	QAction*	a_archive_cleanUp;
	QAction*	a_archive_close;
	QAction*	a_entry_rename;
	QAction*	a_entry_delete;
	QAction*	a_entry_moveUp;
	QAction*	a_entry_moveDown;
	QAction*	a_entry_convertTo;
	QAction*	a_entry_import;
	QAction*	a_entry_export;
	QAction*	a_entry_exportAs;
	QAction*	a_view_toggleArchiveManager;
	QAction*	a_view_toggleConsole;
	QAction*	a_help_onlineDocumentation;
	QAction*	a_help_about;

private slots:
	void	onFileNewWad();
	void	onFileNewZip();
	void	onFileOpen();

public:
	MainWindow();
	~MainWindow();
};

#endif//__MAIN_WINDOW_H__
