#include <qapplication.h>
#include <qwidget.h>
#include <qstringlist.h>
#include <qdir.h>
#include <kio_openwith.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QStringList list;

    list += "file://home/testfile";
    list += "http://www.kde.org/index.html";
    
    KOpenWithDlg* dlg = new KOpenWithDlg(list, "OpenWith_Text", "OpenWith_Value", 0);
    if(dlg->exec()) {
        debug("Dialog ended successfully\ntext: %s", dlg->text().ascii());
    }
    else
        debug("Dialog was cancelled.");
    
    return 0;
}
    
