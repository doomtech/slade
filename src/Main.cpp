
#include "Main.h"
#include "MainWindow.h"
#include <QtGui>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow main_window;
	main_window.show();

    return app.exec();
}
