
#ifndef __FILE_BROWSER_FRAME_H__
#define __FILE_BROWSER_FRAME_H__

#include <QTreeView>

class FileBrowserFrame : public QTreeView {
	Q_OBJECT

private:
	// Actions
	QAction*	a_file_selected;

	// Setup functions
	void	setupActions();

private slots:
	void onItemActivated(const QModelIndex& index);

public:
	FileBrowserFrame(QWidget* parent);
	~FileBrowserFrame();
};

#endif//__FILE_BROWSER_FRAME_H__
