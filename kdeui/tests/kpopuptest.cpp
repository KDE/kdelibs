#include <kapplication.h>
#include <kcmdlineargs.h>
#include <qwidget.h>
#include <qcursor.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPaintEvent>
#include "kmenu.h"

class DemoWidget : public QWidget {
private:
    KMenu *menu;
    
void mousePressEvent(QMouseEvent *)
{
    menu->popup(QCursor::pos());
}

void paintEvent(QPaintEvent *)
{
    drawText(32, 32, "Press a Mouse Button!");
}

public:
    DemoWidget() : QWidget()
    {
        menu = new KMenu("Popup Menu:");
        menu->insertItem("Item1");
        menu->insertItem("Item2");
        menu->insertSeparator();
        menu->insertItem("Quit", qApp, SLOT(quit()));
    }       
};

int main(int argc, char **argv)
{
    KCmdLineArgs::init( argc, argv, "test", "Test" ,"test app" ,"1.0" );
    KApplication app;
    DemoWidget w;
    app.setMainWidget(&w);
    w.setFont(QFont("helvetica", 12, QFont::Bold), true);
    w.show();
    return app.exec();
}
    
