
#ifndef __ARCHIVE_MANAGER_FRAME_H__
#define __ARCHIVE_MANAGER_FRAME_H__

#include <QFrame>
#include <QTabWidget>
#include <QTreeView>

class ArchiveManagerFrame : public QFrame {
	Q_OBJECT
private:
	// Child widgets
	QTabWidget*	w_tabs;
	QTreeView*	w_file_browser;

	// Actions
	QAction*	a_file_selected;

	// Setup functions
	void	setupActions();

private slots:
	void onFBItemActivated(const QModelIndex& index);

public:
	ArchiveManagerFrame(QWidget* parent);
	~ArchiveManagerFrame();
};

#endif//__ARCHIVE_MANAGER_FRAME_H__
