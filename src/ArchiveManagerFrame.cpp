
#include "Main.h"
#include "ArchiveManagerFrame.h"
#include <QtGui>
#include <QBoxLayout>
#include <QFileSystemModel>

ArchiveManagerFrame::ArchiveManagerFrame(QWidget* parent) : QFrame(parent) {
	// Setup layout
	QVBoxLayout* layout = new QVBoxLayout();
	setLayout(layout);

	// Setup file browser
	QFileSystemModel* model = new QFileSystemModel();
	model->setRootPath(QDir::currentPath());
	w_file_browser = new QTreeView();
	w_file_browser->setModel(model);

	// Add tabs
	w_tabs = new QTabWidget(this);
	w_tabs->addTab(w_file_browser, "File Browser");
	w_tabs->addTab(new QTextEdit(), "Archives");
	w_tabs->setCurrentIndex(1);
	layout->addWidget(w_tabs);
	layout->setMargin(4);

	setupActions();
}

ArchiveManagerFrame::~ArchiveManagerFrame() {
}

void ArchiveManagerFrame::setupActions() {
	connect(w_file_browser, SIGNAL(activated(const QModelIndex&)), this, SLOT(onFBItemActivated(const QModelIndex&)));
}

void ArchiveManagerFrame::onFBItemActivated(const QModelIndex& index) {
	QFileSystemModel* model = (QFileSystemModel*)w_file_browser->model();
	string path = model->filePath(index);

	if (!model->isDir(index))
		QMessageBox::information(this, "SLADE", s_fmt("Open file %s", chr(path)));
}
