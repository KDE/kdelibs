#include <kapplication.h>
#include <qwidget.h>
#include <qstringlist.h>
#include <qdir.h>
#include <kopenwith.h>
#include <kurl.h>
#include <kdebug.h>

int main(int argc, char **argv)
{
    KApplication app(argc, argv, "kopenwithtest");
    KURL::List list;

    list += KURL("file://home/testfile");
    list += KURL("http://www.kde.org/index.html");

    KOpenWithDlg* dlg = new KOpenWithDlg(list, "OpenWith_Text", "OpenWith_Value", 0);
    if(dlg->exec()) {
        kdDebug() << "Dialog ended successfully\ntext: " << dlg->text() << endl;
    }
    else
        kdDebug() << "Dialog was cancelled." << endl;

    return 0;
}

