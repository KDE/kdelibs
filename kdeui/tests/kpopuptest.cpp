#include <kapplication.h>
#include <kcmdlineargs.h>
#include <QtGui/QWidget>
#include <QtGui/QCursor>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
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
    QPainter paint(this);
    paint.drawText(32, 32, "Press a Mouse Button!");
}

public:
    DemoWidget() : QWidget()
    {
        menu = new KMenu("Popup Menu:");
        menu->addAction( "Item1" );
        menu->addAction( "Item2" );
        menu->addSeparator();
        QAction *a = new QAction( "Quit", this );
        menu->addAction( a );
        connect( a, SIGNAL(triggered()), qApp, SLOT(quit()));
    }
};

int main(int argc, char **argv)
{
    KCmdLineArgs::init( argc, argv, "test", 0, ki18n("Test"), "1.0", ki18n("test app"));
    KApplication app;
    DemoWidget w;
    w.setFont(QFont("helvetica", 12, QFont::Bold));
    w.show();
    return app.exec();
}
    
