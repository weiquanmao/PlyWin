#include "ply\plywindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	PlyWindow w;
	w.show();
	return a.exec();
}
