#include "mimetypeviewer.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MimeTypeViewer v;
    v.show();

//    MainWindow w;
//    w.show();

    return a.exec();
}
