
#include "Main.h"
#include "FileBrowserFrame.h"
#include <QtGui>
#include <QFileSystemModel>

FileBrowserFrame::FileBrowserFrame(QWidget* parent) : QTreeView(parent) {
	// Setup model
	QFileSystemModel* model = new QFileSystemModel();
	model->setRootPath(QDir::currentPath());
	setModel(model);

	// Setup actions
	setupActions();
}

FileBrowserFrame::~FileBrowserFrame() {
}

void FileBrowserFrame::setupActions() {
	connect(this, SIGNAL(activated(const QModelIndex&)), this, SLOT(onItemActivated(const QModelIndex&)));
}

void FileBrowserFrame::onItemActivated(const QModelIndex& index) {
	QFileSystemModel* model = (QFileSystemModel*)this->model();
	string path = model->filePath(index);

	if (!model->isDir(index))
		QMessageBox::information(this, "SLADE", s_fmt("Open file %s", chr(path)));
}
