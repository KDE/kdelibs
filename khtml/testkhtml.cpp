// programm to test the new khtml implementation


#include "khtmltokenizer.h"
#include "khtmldecoder.h"
#include "kapp.h"
#include "qfile.h"
#include "html_document.h"
#include "khtmltokenizer.h"
// to be able to delete a static protected member pointer in kbrowser...
// just for memory debugging
#define protected public
#include "khtml.h"
#undef protected
#include "qpushbutton.h"
#include "khtmldata.h"
#include "khtmlfont.h"
#include "khtmlio.h"
#include <qcursor.h>

int main(int argc, char *argv[])
{
    char data[1024];

    KApplication a(argc, argv, "testkhtml");

    KHTMLWidget *doc = new KHTMLWidget;
    doc->resize(800,500);
    doc->enableJScript(true);
    doc->enableJava(true);
    //doc->setFollowsLinks(false);

    a.setTopWidget(doc);
    doc->openURL(argv[1]);
    a.setTopWidget(doc);
    doc->setURLCursor(QCursor(PointingHandCursor));
    doc->show();

    QPushButton *p = new QPushButton(0, 0);
    QWidget::connect(p, SIGNAL(pressed()), &a, SLOT(quit()));
    p->show();
    a.exec();

    delete p;
    delete doc;

    if(pSettings) delete pSettings;
    if(pFontManager) delete pFontManager;
    KHTMLCache::clear();
    //if(KBrowser::lstViews) delete KBrowser::lstViews;
}




