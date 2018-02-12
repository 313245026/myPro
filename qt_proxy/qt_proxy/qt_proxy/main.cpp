#include "qt_proxy.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	qt_proxy w;
	w.show();
	return a.exec();
}
